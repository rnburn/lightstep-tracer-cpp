#include <lightstep/tracer.h>
#include "../src/buffered_recorder.h"
#include "../src/lightstep_tracer_impl.h"
#include "in_memory_transporter.h"
#include "span_generator.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

static thread_local std::mt19937 rng{std::random_device()()};

static std::chrono::steady_clock::duration delta = std::chrono::milliseconds(1);

void printTime(const char * what, const std::chrono::steady_clock::time_point& t) {
  auto tse =  t.time_since_epoch();
  auto dur = std::chrono::duration_cast<std::chrono::milliseconds>( tse ).count();
  std::cout << what << "=" << dur << std::endl;
};

TEST_CASE("rpc_recorder") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter();
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION(
      "If spans are recorded at a rate significantly slower than "
      "LightStepTracerOptions::reporting_period, then no spans get dropped.") {
    SpanGenerator span_generator(*tracer, 2 * options.reporting_period);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() ==
          span_generator.num_spans_generated());
  }

  SECTION(
      "If spans are recorded at a rate significantly faster than "
      "LightStepTracerOptions::reporting_period, then a fraction of the spans "
      "get successfuly transported.") {
    SpanGenerator span_generator(*tracer, options.reporting_period / 5);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() >
          span_generator.num_spans_generated() / 10);
  }

  SECTION(
      "If the transporter's SendReport function throws, we drop all subsequent "
      "spans.") {
    SpanGenerator span_generator(*tracer, options.reporting_period * 2);
    in_memory_transporter->set_should_throw(true);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() == 0);
  }
}

struct TestInterface : public BufferedRecorder::TestInterface {
	BufferedRecorder::time_point t_now;
	
	virtual BufferedRecorder::time_point now() { return t_now; }
	
	std::function<bool()> wait_for_cb;

    virtual bool wait_for(
					 std::condition_variable& write_cond_,
					 std::unique_lock<std::mutex>& lock,
					 const BufferedRecorder::duration& rel_time,
					 BufferedRecorder::Predicate pred) {
	  return true;
    }

	std::function<void(const BufferedRecorder::time_point&,
                       std::unique_lock<std::mutex>& lock)> wait_until_cb;

    virtual void wait_until(
		           std::condition_variable& write_cond_,
	               std::unique_lock<std::mutex>& lock,
                   const BufferedRecorder::time_point& timeout_time,
                   BufferedRecorder::Predicate pred ) {
	  wait_until_cb(timeout_time, lock);
    }
	
	void advanceTime() {
		t_now += delta;
	}
};

TEST_CASE("rpc_recorder_step") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter();
  auto ti = std::shared_ptr<TestInterface>(new TestInterface);
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter}, ti};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  std::mutex tick, tock; tick.lock(); tock.lock();
  BufferedRecorder::time_point timeout;
  
  auto defaultCallback = [&]() {
	  ti->wait_until_cb = [&](const BufferedRecorder::time_point& till,
	                          std::unique_lock<std::mutex>&       lock) {
		timeout = till;
		tick.unlock();
		tock.lock();
	  };
  };
  defaultCallback();

  auto tightLoop = [&]() {
      ti->wait_until_cb = [&](const BufferedRecorder::time_point& till,
                              std::unique_lock<std::mutex>&       lock) {
      	std::this_thread::yield(); // A tight loop
      };
  };
  
  auto shutdown = [&]() {
	tick.lock(); // Another timeout
	ti->advanceTime();

	// Done
	tightLoop();

	tock.unlock();
	
	tracer->Close();
  };
  
	  SECTION("Check that the timeout is as expected") {

	for (int i = 0; i < 10; ++i) {
		tick.lock(); // Another timeout

		// This is weird...
		auto expected = ti->t_now + (i+1) * options.reporting_period - i * delta;
		CHECK(timeout == expected);
		ti->advanceTime();
		tock.unlock();
	}

	    shutdown();
	  }
  
  auto sendSpan = [&]{
	auto systemTime = convert_time_point<std::chrono::system_clock>(ti->t_now);
	auto span = std::shared_ptr<opentracing::Span>(
		tracer->StartSpan(std::to_string(rng()),
	                      {opentracing::StartTimestamp(systemTime, ti->t_now)}));
	assert(span);
	return std::thread([&](){
		span->Finish({opentracing::FinishTimestamp(ti->t_now + std::chrono::milliseconds(30))});
	});
  };
  
  SECTION("Single span goes through") {

	tightLoop();

	sendSpan().join();

	    tracer->Close();

	CHECK(in_memory_transporter->spans().size() == 1);
  }
  
  auto step = [&]() {
	tick.lock();
	ti->advanceTime();
	tock.unlock();
  };
  
  std::mutex freed; freed.lock();
  auto toManual = [&]() {
  	ti->wait_until_cb = [&](const BufferedRecorder::time_point& till,
  	                        std::unique_lock<std::mutex>&       lock) {
  		// Release the lock till I tell you.
  		tick.unlock();
  		lock.unlock(); // Free it up for other threads
		freed.unlock();
  		tock.lock();
  		lock.lock(); // Back to being locked.
  	};
  };
  
  SECTION("A span among multiple timeouts survives") {

	step();

	toManual();

	{
		// Send a span
		tick.lock();

		sendSpan().join();

		defaultCallback(); // Restore default
		ti->advanceTime();
		tock.unlock();
	}
	
	step(); // Many timeouts
	step();
	step();
	
	toManual();
	
	{
		// Send many spans quickly
		tick.lock();
		freed.lock(); // Wait till buffer's lock is free
		for (int i = 0; i < 4; ++i) {
			sendSpan().join();
		}
		
		defaultCallback(); // Restore default
		ti->advanceTime();
		tock.unlock();
	}

	step(); // Many timeouts
	step();
	step();

	shutdown();
	
	CHECK(in_memory_transporter->spans().size() == 5);
  }
}
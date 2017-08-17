#include <lightstep/tracer.h>
#include <algorithm>
#include "../src/buffered_recorder.h"
#include "../src/lightstep_tracer_impl.h"
#include "in_memory_transporter.h"
#include "testing_condition_variable_wrapper.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

static int LookupSpansDropped(const collector::ReportRequest& report) {
  if (!report.has_internal_metrics()) {
    return 0;
  }
  auto& counts = report.internal_metrics().counts();
  auto iter = std::find_if(counts.begin(), counts.end(),
                           [](const collector::MetricsSample& sample) {
                             return sample.name() == "spans.dropped";
                           });
  if (iter == counts.end()) {
    return 0;
  }
  if (iter->value_case() != collector::MetricsSample::kIntValue) {
    std::cerr << "spans.dropped not of type int\n";
    std::terminate();
  }
  return static_cast<int>(iter->int_value());
}

TEST_CASE("rpc_recorder") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter{};
  auto condition_variable = new TestingConditionVariableWrapper{};
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter},
      std::unique_ptr<ConditionVariableWrapper>{condition_variable}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION("The writer thread waits until `now() + options.reporting_period`") {
    auto now = condition_variable->Now();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(event->timeout() == now + options.reporting_period);
  }

  SECTION(
      "If the writer thread takes longer than `options.reporting_period` to "
      "run then it runs again immediately upon finishing.") {
    auto now = condition_variable->Now() + 2 * options.reporting_period;
    condition_variable->WaitTillNextEvent();
    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->set_now(now);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(condition_variable->Now() == now);
    CHECK(event->timeout() == now);
  }

  SECTION(
      "If the writer thread takes time between 0 and "
      "`options.reporting_period` to run it subtracts the elapse time from the "
      "next timeout") {
    auto now = condition_variable->Now() + 3 * options.reporting_period / 2;
    condition_variable->WaitTillNextEvent();
    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->set_now(now);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(condition_variable->Now() == now);
    CHECK(event->timeout() == now + options.reporting_period / 2);
  }

  SECTION(
      "If the transporter's SendReport function throws, we drop all subsequent "
      "spans.") {
    logger.set_level(spdlog::level::off);
    in_memory_transporter->set_should_throw(true);

    // Wait until the writer thread is ready to run.
    condition_variable->WaitTillNextEvent();

    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->Step();

    // Create another span
    span = tracer->StartSpan("abc");
    span->Finish();

    tracer->Close();

    // Verify no spans were transported.
    CHECK(in_memory_transporter->spans().size() == 0);
  }

  SECTION(
      "Dropped spans counts get sent in the next ReportRequest, and cleared in "
      "the following ReportRequest") {
    condition_variable->set_block_notify_all(true);
    for (size_t i = 0; i < options.max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    // Check that a NotifyAllEvent gets added when the buffer overflows
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::NotifyAllEvent*>(
              condition_variable->next_event()) != nullptr);

    // Wait until the first report gets sent
    condition_variable->Step();
    condition_variable->Step();
    condition_variable->set_block_notify_all(false);

    auto span = tracer->StartSpan("xyz");
    CHECK(span);
    span->Finish();
    // Ensure that the second report gets sent.
    condition_variable->Step();
    condition_variable->Step();

    auto reports = in_memory_transporter->reports();
    CHECK(reports.size() == 2);
    CHECK(LookupSpansDropped(reports.at(0)) == 1);
    CHECK(reports.at(0).spans_size() == options.max_buffered_spans);
    CHECK(LookupSpansDropped(reports.at(1)) == 0);
    CHECK(reports.at(1).spans_size() == 1);
  }
}

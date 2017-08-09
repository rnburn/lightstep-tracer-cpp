#pragma once

#include <collector.pb.h>
#include <lightstep/spdlog/logger.h>
#include <lightstep/tracer.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include "recorder.h"
#include "report_builder.h"
#include "transporter.h"

namespace lightstep {
/**
 * BufferedRecorder buffers spans finished by a tracer and sends them over to
 * the provided Transporter.
 */
class BufferedRecorder : public Recorder {
 public:
  typedef std::chrono::steady_clock::duration duration;
  typedef std::chrono::steady_clock::time_point time_point;
  typedef std::function<bool()> Predicate;

  struct TestInterface {
	  virtual bool wait_for(
		                 std::condition_variable& write_cond_,
		                 std::unique_lock<std::mutex>& lock,
		                 const duration& rel_time,
		                 Predicate pred ) = 0;

	  virtual void wait_until(
		               std::condition_variable& write_cond_,
		               std::unique_lock<std::mutex>& lock,
	                   const time_point& timeout_time,
	                   Predicate pred ) = 0;

	  virtual time_point now() = 0;
  };
  
 private:
    struct DefaultTestInterface : public TestInterface {
  	    virtual bool wait_for(
						 std::condition_variable& write_cond_,
  		                 std::unique_lock<std::mutex>& lock,
  		                 const duration& rel_time,
  		                 Predicate pred );

  	    virtual void wait_until(
			           std::condition_variable& write_cond_,
  		               std::unique_lock<std::mutex>& lock,
  	                   const time_point& timeout_time,
  	                   Predicate pred );

	  virtual time_point now();
    };

 public:
  
  BufferedRecorder(spdlog::logger& logger, LightStepTracerOptions options,
                   std::unique_ptr<Transporter>&& transporter,
				   const std::shared_ptr<TestInterface>& testInterface = 
				       std::shared_ptr<TestInterface>(new DefaultTestInterface));

  BufferedRecorder(const BufferedRecorder&) = delete;
  BufferedRecorder(BufferedRecorder&&) = delete;
  BufferedRecorder& operator=(const BufferedRecorder&) = delete;
  BufferedRecorder& operator=(BufferedRecorder&&) = delete;

  ~BufferedRecorder() override;

  void RecordSpan(collector::Span&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  void Write() noexcept;
  bool WriteReport(const collector::ReportRequest& report);
  void FlushOne();

  // Forces the writer thread to exit immediately.
  void MakeWriterExit();

  // Waits until either the timeout or the writer thread is forced to
  // exit.  Returns true if it should continue writing, false if it
  // should exit.
  bool WaitForNextWrite(const std::chrono::steady_clock::time_point& next);

  spdlog::logger& logger_;
  const LightStepTracerOptions options_;

  // Writer state.
  std::mutex write_mutex_;
  std::condition_variable write_cond_;
  bool write_exit_ = false;
  std::thread writer_;

  // Buffer state (protected by write_mutex_).
  ReportBuilder builder_;
  collector::ReportRequest inflight_;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;

  // Transporter through which to send span reports.
  std::unique_ptr<Transporter> transporter_;
  
  std::shared_ptr<TestInterface> testInterface_;
};
}  // namespace lightstep

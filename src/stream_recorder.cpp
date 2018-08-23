#include "stream_recorder.h"

#include "lightstep-tracer-common/collector.pb.h"
#include "utility.h"

#include <chrono>
#include <stdexcept>

namespace lightstep {
static const std::chrono::steady_clock::duration polling_interval =
    std::chrono::microseconds{10};
//------------------------------------------------------------------------------
// MakeStreamInitializationMessage
//------------------------------------------------------------------------------
static collector::StreamInitialization MakeStreamInitializationMessage(
    const LightStepTracerOptions& options) {
  collector::StreamInitialization initialization;
  initialization.set_reporter_id(GenerateId());
  auto& tags = *initialization.mutable_tags();
  tags.Reserve(static_cast<int>(options.tags.size()));
  for (const auto& tag : options.tags) {
    *tags.Add() = ToKeyValue(tag.first, tag.second);
  }
  return initialization;
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamRecorder::StreamRecorder(Logger& logger, LightStepTracerOptions&& options,
                               std::unique_ptr<StreamTransporter>&& transporter)
    : logger_{logger},
      transporter_{std::move(transporter)},
      message_buffer_{options.message_buffer_size} {
  streamer_thread_ = std::thread{&StreamRecorder::RunStreamer, this};
  if (!message_buffer_.Add(MakeStreamInitializationMessage(options))) {
    throw std::runtime_error{"buffer size is too small"};
  }
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamRecorder::~StreamRecorder() {
  MakeStreamerExit();
  streamer_thread_.join();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void StreamRecorder::RecordSpan(const collector::Span& span) noexcept {
  message_buffer_.Add(span);
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool StreamRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {
  return true;
}

//------------------------------------------------------------------------------
// RunStreamer
//------------------------------------------------------------------------------
void StreamRecorder::RunStreamer() noexcept try {
  while (SleepForNextPoll()) {
    while (1) {
      if (!message_buffer_.Consume(StreamRecorder::Consume,
                                   static_cast<void*>(this))) {
        break;
      }
    }
  }
} catch (const std::exception& /*e*/) {
  MakeStreamerExit();
}

//------------------------------------------------------------------------------
// MakeStreamerExit
//------------------------------------------------------------------------------
void StreamRecorder::MakeStreamerExit() noexcept {
  {
    std::unique_lock<std::mutex> lock{mutex_};
    exit_streamer_ = true;
  }
  condition_variable_.notify_all();
}

//------------------------------------------------------------------------------
// StreamRecorder
//------------------------------------------------------------------------------
bool StreamRecorder::SleepForNextPoll() {
  std::unique_lock<std::mutex> lock{mutex_};
  return !condition_variable_.wait_for(lock, polling_interval,
                                       [this] { return this->exit_streamer_; });
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
size_t StreamRecorder::Consume(void* context, const char* data,
                               size_t num_bytes) {
  auto& stream_recorder = *static_cast<StreamRecorder*>(context);
  return stream_recorder.transporter_->Write(data, num_bytes);
}
}  // namespace lightstep

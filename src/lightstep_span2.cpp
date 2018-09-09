#include "lightstep_span2.h"

#include "utility.h"
#include "serialization.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <cstdlib>
#include <memory>
#include <iterator>
#include <algorithm>

using opentracing::SystemTime;
using opentracing::SystemClock;
using opentracing::SteadyClock;
using opentracing::SteadyTime;

using ProtoSpan = lightstep::collector::Span;
using ProtoSpanContext = lightstep::collector::SpanContext;

namespace lightstep {
const size_t DefaultInitialBlockSize = 512;

//------------------------------------------------------------------------------
// ComputeStartTimestamps
//------------------------------------------------------------------------------
static std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{SystemClock::now(),
                                              SteadyClock::now()};
  }
  if (start_system_timestamp == SystemTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        opentracing::convert_time_point<SystemClock>(start_steady_timestamp),
        start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        start_system_timestamp,
        opentracing::convert_time_point<SteadyClock>(start_system_timestamp)};
  }
  return std::tuple<SystemTime, SteadyTime>{start_system_timestamp,
                                            start_steady_timestamp};
}

//------------------------------------------------------------------------------
// operator new
//------------------------------------------------------------------------------
void* LightStepSpan2::operator new(size_t /*size*/) {
  static_assert(
      DefaultInitialBlockSize > sizeof(LightStepSpan2),
      "Initial block size must be large enough to contain LightStepSpan");
  constexpr auto alignment = alignof(LightStepSpan2);
  void* result;
  auto rcode = posix_memalign(&result, alignment, DefaultInitialBlockSize);
  if (rcode != 0) {
    throw std::bad_alloc{};
  }
  return result;
}

//------------------------------------------------------------------------------
// operator delete
//------------------------------------------------------------------------------
void LightStepSpan2::operator delete(void * ptr) {
  free(ptr);
}

//------------------------------------------------------------------------------
// GetArenaOptions
//------------------------------------------------------------------------------
google::protobuf::ArenaOptions LightStepSpan2::GetArenaOptions() {
  google::protobuf::ArenaOptions result;
  result.initial_block = reinterpret_cast<char*>(this) + sizeof(LightStepSpan2);
  result.initial_block_size = DefaultInitialBlockSize - sizeof(LightStepSpan2);
  return result;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepSpan2::LightStepSpan2(
    std::shared_ptr<const opentracing::Tracer>&& tracer, Logger& logger,
    Recorder& recorder, opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options)
    : arena_{GetArenaOptions()},
      tracer_{std::move(tracer)},
      logger_{logger},
      recorder_{recorder} {
  // Set operation name.
  auto operation_name_data = google::protobuf::Arena::CreateArray<char>(
      &arena_, operation_name.size());
  std::copy(std::begin(operation_name), std::end(operation_name),
            operation_name_data);
  operation_name_ =
      opentracing::string_view{operation_name_data, operation_name.size()};

  // Set the start timestamps.
  std::chrono::system_clock::time_point start_system_timestamp;
  std::tie(start_system_timestamp, start_steady_timestamp_) =
      ComputeStartTimestamps(options.start_system_timestamp,
                             options.start_steady_timestamp);
  std::tie(start_timestamp_seconds_since_epoch_,
           start_timestamp_nano_fraction_) =
      ProtobufFormatTimestamp(start_system_timestamp);

  // Set IDs.
  trace_id_ = GenerateId();
  span_id_ = GenerateId();
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
LightStepSpan2::~LightStepSpan2() {
  if (!is_finished_) {
    Finish();
  }
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void LightStepSpan2::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  auto finish_timestamp = options.finish_steady_timestamp;
  if (finish_timestamp == SteadyTime()) {
    finish_timestamp = SteadyClock::now();
  }

  // Set timing information.
  auto duration = finish_timestamp - start_steady_timestamp_;
  duration_micros_ =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
} catch (const std::exception& e) {
  logger_.Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void LightStepSpan2::SetOperationName(
    opentracing::string_view name) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LightStepSpan2::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan2::SetBaggageItem(
    opentracing::string_view restricted_key,
    opentracing::string_view value) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetBaggageItem failed: ", e.what());
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LightStepSpan2::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
} catch (const std::exception& e) {
  logger_.Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void LightStepSpan2::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan2::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
}

//------------------------------------------------------------------------------
// sampled
//------------------------------------------------------------------------------
bool LightStepSpan2::sampled() const noexcept {
  return true;
}

//------------------------------------------------------------------------------
// ComputeSpanContextSerializationSize
//------------------------------------------------------------------------------
void LightStepSpan2::ComputeSpanContextSerializationSize() {
  span_context_serialization_size_ =
      ComputeVarintSerializationSize<ProtoSpanContext::kTraceIdFieldNumber>(
          trace_id_) +
      ComputeVarintSerializationSize<ProtoSpanContext::kSpanIdFieldNumber>(
          span_id_);
}

//------------------------------------------------------------------------------
// ComputeStartTimestampSerializationSize
//------------------------------------------------------------------------------
void LightStepSpan2::ComputeStartTimestampSerializationSize() {
  start_timestamp_serialization_size_ =
      ComputeVarintSerializationSize<1>(start_timestamp_seconds_since_epoch_) +
      ComputeVarintSerializationSize<2>(start_timestamp_nano_fraction_);
}

//------------------------------------------------------------------------------
// ComputeSerializationSizes
//------------------------------------------------------------------------------
void LightStepSpan2::ComputeSerializationSizes() {
  ComputeSpanContextSerializationSize();
  ComputeStartTimestampSerializationSize();
  serialization_size_ =
      // span_context
      ComputeLengthDelimitedSerializationSize<
          ProtoSpan::kSpanContextFieldNumber>(
          span_context_serialization_size_) +
      // operation_name
      ComputeLengthDelimitedSerializationSize<
          ProtoSpan::kOperationNameFieldNumber>(operation_name_.size()) +
      // start_timestamp
      ComputeLengthDelimitedSerializationSize<
          ProtoSpan::kStartTimestampFieldNumber>(
          start_timestamp_serialization_size_) +
      // duration_micros
      ComputeVarintSerializationSize<ProtoSpan::kDurationMicrosFieldNumber>(
          duration_micros_);
}

//------------------------------------------------------------------------------
// SerializeSpanContext
//------------------------------------------------------------------------------
void LightStepSpan2::SerializeSpanContext(
    google::protobuf::io::CodedOutputStream& ostream) {
  SerializeVarint<ProtoSpanContext::kTraceIdFieldNumber>(ostream, trace_id_);
  SerializeVarint<ProtoSpanContext::kSpanIdFieldNumber>(ostream, span_id_);
}

//------------------------------------------------------------------------------
// LightStepSpan2
//------------------------------------------------------------------------------
void LightStepSpan2::SerializeStartTimestamp(
    google::protobuf::io::CodedOutputStream& ostream) {
  SerializeVarint<1>(ostream, start_timestamp_seconds_since_epoch_);
  SerializeVarint<2>(ostream, start_timestamp_nano_fraction_);
}

//------------------------------------------------------------------------------
// Serialize
//------------------------------------------------------------------------------
void LightStepSpan2::Serialize(
    google::protobuf::io::CodedOutputStream& ostream) {
  // span context
  SerializeKeyLength<ProtoSpan::kSpanContextFieldNumber>(
      ostream, span_context_serialization_size_);
  SerializeSpanContext(ostream);

  // operation_name
  SerializeString<ProtoSpan::kOperationNameFieldNumber>(ostream,
                                                        operation_name_);

  // start_timestamp
  SerializeKeyLength<ProtoSpan::kStartTimestampFieldNumber>(
      ostream, start_timestamp_serialization_size_);
  SerializeStartTimestamp(ostream);

  // duration_micros
  SerializeVarint<ProtoSpan::kDurationMicrosFieldNumber>(ostream,
                                                         duration_micros_);
}

void LightStepSpan2::Serialize(
    void* context, google::protobuf::io::CodedOutputStream& ostream) {
  static_cast<LightStepSpan2*>(context)->Serialize(ostream);
}
} // namespace lightstep

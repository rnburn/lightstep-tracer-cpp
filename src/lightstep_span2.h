#pragma once

#include "logger.h"
#include "lightstep_span_context.h"
#include "recorder.h"

#include <opentracing/span.h>

#include <google/protobuf/arena.h>

namespace lightstep {
class LightStepSpan2 final : public opentracing::Span,
                            public LightStepSpanContext {
 public:
  void* operator new(size_t size);

  void operator delete(void * ptr);

  LightStepSpan2(std::shared_ptr<const opentracing::Tracer>&& tracer,
                 Logger& logger, Recorder& recorder,
                 opentracing::string_view operation_name,
                 const opentracing::StartSpanOptions& options);

  LightStepSpan2(const LightStepSpan2&) = delete;
  LightStepSpan2(LightStepSpan2&&) = delete;
  LightStepSpan2& operator=(const LightStepSpan2&) = delete;
  LightStepSpan2& operator=(LightStepSpan2&&) = delete;

  ~LightStepSpan2() override;

  void FinishWithOptions(
      const opentracing::FinishSpanOptions& options) noexcept override;

  void SetOperationName(opentracing::string_view name) noexcept override;

  void SetTag(opentracing::string_view key,
              const opentracing::Value& value) noexcept override;

  void SetBaggageItem(opentracing::string_view restricted_key,
                      opentracing::string_view value) noexcept override;

  std::string BaggageItem(opentracing::string_view restricted_key) const
      noexcept override;

  void Log(std::initializer_list<
           std::pair<opentracing::string_view, opentracing::Value>>
               fields) noexcept override;

  const opentracing::SpanContext& context() const noexcept override {
    return *this;
  }

  const opentracing::Tracer& tracer() const noexcept override {
    return *tracer_;
  }

  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override;

  bool sampled() const noexcept override;

  uint64_t trace_id() const noexcept override {
    return 0;
  }

  uint64_t span_id() const noexcept override {
    return 0;
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      std::ostream& writer) const override {
    /* return this->InjectImpl(propagation_options, writer); */
    return {};
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::TextMapWriter& writer) const override {
    /* return this->InjectImpl(propagation_options, writer); */
    return {};
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::HTTPHeadersWriter& writer) const override {
    /* return this->InjectImpl(propagation_options, writer); */
    return {};
  }

 private:
  google::protobuf::Arena arena_;
  std::shared_ptr<const opentracing::Tracer> tracer_;
  Logger& logger_;
  Recorder& recorder_;

  std::atomic<bool> is_finished_{false};

  uint64_t trace_id_;
  uint64_t span_id_;

  opentracing::string_view operation_name_;

  uint64_t start_timestamp_seconds_since_epoch_;
  uint32_t start_timestamp_nano_fraction_;

  std::chrono::steady_clock::time_point start_steady_timestamp_;
  uint64_t duration_micros_;

  size_t span_context_serialization_size_;
  size_t start_timestamp_serialization_size_;
  size_t serialization_size_;

  google::protobuf::ArenaOptions GetArenaOptions();

  void ComputeSpanContextSerializationSize();
  void ComputeStartTimestampSerializationSize();
  void ComputeSerializationSizes();

  void SerializeSpanContext(google::protobuf::io::CodedOutputStream& ostream);
  void SerializeStartTimestamp(google::protobuf::io::CodedOutputStream& ostream);

  void Serialize(google::protobuf::io::CodedOutputStream& ostream);

  static void Serialize(void* context,
                        google::protobuf::io::CodedOutputStream& ostream);
};
} // namespace lightstep

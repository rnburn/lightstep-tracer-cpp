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

  google::protobuf::ArenaOptions GetArenaOptions();
};
} // namespace lightstep

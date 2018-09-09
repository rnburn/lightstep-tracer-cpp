#include "lightstep_tracer_impl2.h"
#include "lightstep_immutable_span_context.h"
#include "lightstep_span2.h"

namespace lightstep {

//------------------------------------------------------------------------------
// InjectImpl
//------------------------------------------------------------------------------
template <class Carrier>
static opentracing::expected<void> InjectImpl(
    const PropagationOptions& propagation_options,
    const opentracing::SpanContext& span_context, Carrier& writer) {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  return lightstep_span_context->Inject(propagation_options, writer);
}

//------------------------------------------------------------------------------
// ExtractImpl
//------------------------------------------------------------------------------
template <class Carrier>
opentracing::expected<std::unique_ptr<opentracing::SpanContext>> ExtractImpl(
    const PropagationOptions& propagation_options, Carrier& reader) try {
  uint64_t trace_id, span_id;
  bool sampled;
  BaggageMap baggage;
  auto extract_maybe = ExtractSpanContext(propagation_options, reader, trace_id,
                                          span_id, sampled, baggage);
  if (!extract_maybe) {
    return opentracing::make_unexpected(extract_maybe.error());
  }
  if (!*extract_maybe) {
    return std::unique_ptr<opentracing::SpanContext>{nullptr};
  }
  std::unique_ptr<opentracing::SpanContext> result{
      new LightStepImmutableSpanContext{trace_id, span_id, sampled,
                                        std::move(baggage)}};
  return std::move(result);
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepTracerImpl2::LightStepTracerImpl2(
    const PropagationOptions& propagation_options,
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::make_shared<Logger>()},
      propagation_options_{propagation_options},
      recorder_{std::move(recorder)} {}

LightStepTracerImpl2::LightStepTracerImpl2(
    std::shared_ptr<Logger> logger,
    const PropagationOptions& propagation_options,
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::move(logger)},
      propagation_options_{propagation_options},
      recorder_{std::move(recorder)} {}

//------------------------------------------------------------------------------
// StartSpanWithOptions
//------------------------------------------------------------------------------
std::unique_ptr<opentracing::Span> LightStepTracerImpl2::StartSpanWithOptions(
    opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options) const noexcept try {
  return std::unique_ptr<opentracing::Span>{new LightStepSpan2{
      shared_from_this(), *logger_, *recorder_, operation_name, options}};
} catch (const std::exception& e) {
  logger_->Error("StartSpanWithOptions failed: ", e.what());
  return nullptr;
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
opentracing::expected<void> LightStepTracerImpl2::Inject(
    const opentracing::SpanContext& span_context, std::ostream& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl2::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::TextMapWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl2::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::HTTPHeadersWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl2::Extract(std::istream& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl2::Extract(const opentracing::TextMapReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl2::Extract(
    const opentracing::HTTPHeadersReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

//------------------------------------------------------------------------------
// Flush
//------------------------------------------------------------------------------
bool LightStepTracerImpl2::Flush() noexcept {
  return recorder_->FlushWithTimeout(std::chrono::hours(24));
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void LightStepTracerImpl2::Close() noexcept { Flush(); }
}  // namespace lightstep


#pragma once

#include "tracer/propagator.h"

namespace lightstep {
class MultiheaderPropagator : public Propagator {
 public:
  MultiheaderPropagator(opentracing::string_view trace_id_key,
                        opentracing::string_view span_id_key,
                        opentracing::string_view sampled_key,
                        opentracing::string_view baggage_prefix) noexcept;

  // Propagator
  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageProtobufMap& baggage) const override;

  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageFlatMap& baggage) const override;

  opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
      bool& sampled, BaggageProtobufMap& baggage) const override;

 private:
  opentracing::string_view trace_id_key_;
  opentracing::string_view span_id_key_;
  opentracing::string_view sampled_key_;
  opentracing::string_view baggage_prefix_;

  template <class BaggageMap>
  opentracing::expected<void> InjectSpanContextImpl(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageMap& baggage) const;
};
}  // namespace lightstep

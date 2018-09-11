#pragma once

#include "baggage.h"
#include "logger.h"

#include "lightstep-tracer-common/collector.pb.h"

#include <opentracing/span.h>
#include <google/protobuf/arena.h>

#include <cstdint>


namespace lightstep {
struct SpanReference {
    collector::Reference_Relationship reference_type;
    uint64_t trace_id;
    uint64_t span_id;
    size_t span_context_serialization_size;
};

struct SpanReferences {
  const SpanReference* data;
  size_t num_references;
  size_t serialization_size;
};

/* class SpanReferences { */
/*  public: */

/*  private: */
/* }; */

//------------------------------------------------------------------------------
// SetSpanReferences
//------------------------------------------------------------------------------
const SpanReferences* SetSpanReferences(
    Logger& logger, google::protobuf::Arena& arena,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>* reference_iter,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>* reference_last,
    Baggage& baggage, bool& sampled);

} // namespace lightstep

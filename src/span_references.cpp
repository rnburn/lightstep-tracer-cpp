#include "span_references.h"

#include "lightstep_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// SetSpanReference
//------------------------------------------------------------------------------
static bool SetSpanReference(
    Logger& logger,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>& reference_in,
    Baggage& baggage, bool& sampled, SpanReference& reference_out) {
  if (reference_in.second == nullptr) {
    logger.Warn("Passed in null span reference.");
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference_in.second);
  if (referenced_context == nullptr) {
    logger.Warn("Passed in span reference of unexpected type.");
    return false;
  }
  switch (reference_in.first) {
    case opentracing::SpanReferenceType::ChildOfRef:
      reference_out.reference_type = collector::Reference::CHILD_OF;
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      reference_out.reference_type = collector::Reference::FOLLOWS_FROM;
      break;
  }
  reference_out.trace_id = referenced_context->trace_id();
  reference_out.span_id = referenced_context->span_id();
  sampled = sampled || referenced_context->sampled();

  // TODO: Merge in baggage items
  (void)baggage;

  return true;
}

//------------------------------------------------------------------------------
// SetSpanReferences
//------------------------------------------------------------------------------
const SpanReferences* SetSpanReferences(
    Logger& logger, google::protobuf::Arena& arena,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>* reference_iter,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>* reference_last,
    Baggage& baggage, bool& sampled) {
  if (reference_iter == reference_last) {
    return nullptr;
  }
  auto result = google::protobuf::Arena::Create<SpanReferences>(&arena);
  auto capacity = std::distance(reference_iter, reference_last);
  auto references = 
      google::protobuf::Arena::CreateArray<SpanReference>(&arena, capacity);
  result->data = references;
  result->num_references = 0;

  sampled = false;
  for (; reference_iter!=reference_last; ++reference_iter) {
    if (!SetSpanReference(logger, *reference_iter, baggage, sampled,
                          references[result->num_references])) {
      continue;
    }
    ++result->num_references;
  }

  if (result->num_references == 0) {
    // If all references are invalid, act as if there are no references.
    return nullptr;
  }

  return result;
}
} // namespace lightstep

#include "recorder/stream_recorder/embedded_metrics_message.h"

#include "common/protobuf.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
EmbeddedMetricsMessage::EmbeddedMetricsMessage()
    : dropped_spans_count_{*message_.add_counts()} {
  dropped_spans_count_.set_name("spans.dropped");
}

//--------------------------------------------------------------------------------------------------
// set_num_dropped_spans
//--------------------------------------------------------------------------------------------------
void EmbeddedMetricsMessage::set_num_dropped_spans(int num_dropped_spans) const
    noexcept {
  dropped_spans_count_.set_int_value(num_dropped_spans);
}

//--------------------------------------------------------------------------------------------------
// num_dropped_spans
//--------------------------------------------------------------------------------------------------
int EmbeddedMetricsMessage::num_dropped_spans() const noexcept {
  return dropped_spans_count_.int_value();
}

//--------------------------------------------------------------------------------------------------
// MakeFragment
//--------------------------------------------------------------------------------------------------
std::pair<void*, int> EmbeddedMetricsMessage::MakeFragment() {
  buffer_.resize(ComputeEmbeddedMessageSerializationSize(
      collector::ReportRequest::kInternalMetricsFieldNumber, message_));
  {
    google::protobuf::io::ArrayOutputStream zero_copy_stream{
        static_cast<void*>(buffer_.data()), static_cast<int>(buffer_.size())};
    google::protobuf::io::CodedOutputStream coded_stream{&zero_copy_stream};
    WriteEmbeddedMessage(coded_stream,
                         collector::ReportRequest::kInternalMetricsFieldNumber,
                         message_);
  }
  return {static_cast<void*>(buffer_.data()), static_cast<int>(buffer_.size())};
}
}  // namespace lightstep

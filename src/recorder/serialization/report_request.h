#pragma once

#include <memory>

#include "lightstep/buffer_chain.h"
#include "recorder/serialization/embedded_metrics_message.h"
#include "common/serialization_chain.h"

namespace lightstep {
class ReportRequest final : public BufferChain {
 public:
  ReportRequest(const std::shared_ptr<const std::string>& header,
                std::unique_ptr<EmbeddedMetricsMessage>&& metrics);

  // BufferChain
  size_t num_fragments() const noexcept override;

  size_t num_bytes() const noexcept override;

  bool ForEachFragment(FragmentCallback callback, void* context) const override;

 private:
  std::shared_ptr<const std::string> header_;

  std::unique_ptr<const EmbeddedMetricsMessage> metrics_;
  std::pair<void*, int> metrics_fragment_;

  int num_spans_{0};
  std::unique_ptr<SerializationChain> spans_;
};
} // namespace lightstep
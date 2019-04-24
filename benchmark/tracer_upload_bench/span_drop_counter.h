#pragma once

#include <atomic>

#include "lightstep/metrics_observer.h"

namespace lightstep {
class SpanDropCounter : public MetricsObserver {
 public:
   int num_dropped_spans() const noexcept { return num_dropped_spans_; }

   // MetricsObserver
  void OnSpansDropped(int num_spans) override {
    num_dropped_spans_ += num_spans;
  }
 private:
   std::atomic<int> num_dropped_spans_{0};
};
} // namespace lightstep

#include "benchmark_report.h"

#include <iostream>

namespace lightstep {
std::ostream& operator<<(std::ostream& out, const BenchmarkReport& report) {
  std::cout << "Total spans: " << report.num_spans_generated << "\n";
  std::cout << "Dropped spans: " << report.num_dropped_spans << "\n";
  auto num_sent_spans =  report.num_spans_generated - report.num_dropped_spans;
  auto upload_rate = 1e6 * static_cast<double>(num_sent_spans) / report.duration.count();
  std::cout << "Upload rate (spans/sec): " << upload_rate << "\n";
  return out;
}
} // namespace lightstep

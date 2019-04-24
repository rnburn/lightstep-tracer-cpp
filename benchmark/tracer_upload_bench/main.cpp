#include "lightstep/tracer.h"

#include <iostream>
#include <chrono>

#include "utility.h"
#include "span.h"
using namespace lightstep;

int main(int argc, char* argv[]) try { 
  if (argc != 2) {
    std::cout << "Usage: span-upload-bench <benchmark_configuration>\n";
    return 1;
  }
  auto config = ParseConfiguration(argv[1]);
  auto tracer = MakeTracer(config);
  auto t1 = std::chrono::steady_clock::now();
  GenerateSpans(*tracer, config);
  auto t2 = std::chrono::steady_clock::now();
  std::cout
      << "duration = "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
      << "\n";
  return 0;
} catch (const std::exception& e) {
  std::cerr << e.what() << "\n";
  return -1;
}


#include "lightstep/tracer.h"

#include <iostream>

#include "utility.h"
using namespace lightstep;

int main(int argc, char* argv[]) try { 
  if (argc != 2) {
    std::cout << "Usage: span-upload-bench <benchmark_configuration>\n";
    return 1;
  }
  auto config = ParseConfiguration(argv[1]);
  auto tracer = MakeTracer(config);
  auto span = tracer->StartSpan("abc");
  span->Finish();
  tracer->Close();
  return 0; 
} catch (const std::exception& e) {
  std::cerr << e.what() << "\n";
  return -1;
}


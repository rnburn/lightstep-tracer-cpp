#include <lightstep/tracer.h>
#include <cassert>
#include <chrono>
#include <cstdlib>  // for std::getenv
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static void MakeSpan(opentracing::Tracer& tracer, int span_index) {
  /* std::cout << "Make Span: " << span_index << "\n"; */
  auto span = tracer.StartSpan("span_" + std::to_string(span_index));
  assert(span != nullptr);
  for (int tag_index = 0; tag_index < 25; ++tag_index) {
    span->SetTag("tag_" + std::to_string(tag_index), tag_index);
  }
}

int main() {
  lightstep::LightStepTracerOptions options;
  options.collector_plaintext = true;
  options.satellite_endpoints = {{"collector.lightstep.com", 80}};
  options.use_stream_recorder = true;
  /* options.verbose = true; */
  options.component_name = "Stream";
  if (const char* access_token = std::getenv("LIGHTSTEP_ACCESS_TOKEN")) {
    options.access_token = access_token;
  } else {
    std::cerr << "You must set the environmental variable "
                 "`LIGHTSTEP_ACCESS_TOKEN` to your access token!\n";
    return -1;
  }

  auto tracer = MakeLightStepTracer(std::move(options));
  assert(tracer != nullptr);
  auto t1 = std::chrono::system_clock::now();
  for (int i = 0; i < 1000; ++i) {
    MakeSpan(*tracer, i);
  }
  auto t2 = std::chrono::system_clock::now();
  (void)t2;
  /* std::cout << "Closing tracer\n"; */
  tracer->Close();
  auto t3 = std::chrono::system_clock::now();
  std::cout << "Total duration (milliseconds): " << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1).count() << "\n";
  return 0;
}

#pragma once

#include <lightstep/tracer.h>
#include <chrono>
#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
// Abstract class that accepts spans from a Tracer once they are finished.
class Recorder {
 public:
  using SerializationFunction =
      void (*)(void* context, google::protobuf::io::CodedOutputStream& ostream);

  virtual ~Recorder() = default;

  virtual void RecordSpan(const collector::Span& span) noexcept = 0;

  virtual void RecordSpan(SerializationFunction serialization_function,
                          void* context, size_t size) {
  }

  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }


};
}  // namespace lightstep

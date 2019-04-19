#pragma once

#include "benchmark/tracer_upload_bench/tracer_upload_bench.pb.h"

#include "lightstep/tracer.h"

namespace lightstep {
tracer_upload_bench::Configuration ParseConfiguration(const char* filename);

std::shared_ptr<LightStepTracer> MakeTracer(const tracer_upload_bench::Configuration& config);
} // namespace lightstep

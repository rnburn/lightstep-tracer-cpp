#include <lightstep/tracer.h>
#include <cstring>
using namespace lightstep;

extern "C" int __attribute((weak))
opentracing_make_vendor_tracer(const char* opentracing_version,
                               const char* json_configuration,
                               const char** error_message,
                               void* tracer_shared_ptr) {
  // TODO: Allow versions to differ in minor?
  if (std::strcmp(opentracing_version, OPENTRACING_VERSION) != 0) {
    *error_message = "incompatible versions of opentracing";
    return -1;
  }
  auto options = LightStepTracerOptions::FromJson(json_configuration);
  if (!options) {
    *error_message = "invalid configuration";
    return -1;
  }
  *static_cast<std::shared_ptr<opentracing::Tracer>*>(tracer_shared_ptr) =
      MakeLightStepTracer(std::move(*options));
  return 0;
}

#include "utility.h"

#include <fstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <iterator>

#include "google/protobuf/util/json_util.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ReadFile
//--------------------------------------------------------------------------------------------------
static std::string ReadFile(const char* filename) {
  std::ifstream in{filename};
  if (!in.good()) {
    std::ostringstream oss;
    oss << "Failed to open " << filename << ": " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  return std::string{std::istreambuf_iterator<char>{in},
                     std::istreambuf_iterator<char>{}};
}

//--------------------------------------------------------------------------------------------------
// ParseConfiguration
//--------------------------------------------------------------------------------------------------
tracer_upload_bench::Configuration ParseConfiguration(const char* filename) {
  auto config_json = ReadFile(filename);
  tracer_upload_bench::Configuration result;
  auto parse_result =
      google::protobuf::util::JsonStringToMessage(config_json, &result);
  if (!parse_result.ok()) {
    std::ostringstream oss;
    oss << "Failed to parse configuration json: " << parse_result.ToString();
    throw std::runtime_error{oss.str()};
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
std::shared_ptr<LightStepTracer> MakeTracer(
    const tracer_upload_bench::Configuration& config) {
  std::string tracer_config_json;
  auto convert_result = google::protobuf::util::MessageToJsonString(
      config.tracer_configuration(), &tracer_config_json);
  if (!convert_result.ok()) {
    std::ostringstream oss;
    oss << "Failed to construct tracer configuration: "
        << convert_result.ToString();
    throw std::runtime_error{oss.str()};
  }
  return MakeLightStepTracer(tracer_config_json.c_str());
}
} // namespace lightstep

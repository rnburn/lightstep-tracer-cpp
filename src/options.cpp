#include "options.h"
#include "utility.h"
#include <lightstep/nlohmann/json.hpp>
#include <string>

namespace lightstep {
//------------------------------------------------------------------------------
// get_integer
//------------------------------------------------------------------------------
static json::number_integer_t get_integer(const json& value) {
  if (value.is_string()) try {
    return std::stoi(value.get_ref<const std::string&>());
  } catch(const std::invalid_argument& /*error*/) {
    throw std::domain_error{"not a valid integer"};
  } catch (const std::out_of_range& /*error*/) {
    throw std::domain_error{"out of range"};
  }
  return value.get<json::number_integer_t>();
}

//------------------------------------------------------------------------------
// get_bool
//------------------------------------------------------------------------------
static json::boolean_t get_bool(const json& value) {
  if (value.is_string()) {
    const json::string_t& s = value.get_ref<const json::string_t&>();
    if (s == "0" || iequals(s, "false")) {
      return false;
    } else if (s == "1" || iequals(s, "true")) {
      return true;
    } else {
      throw std::domain_error{"not a valid bool"};
    }
  }
  return value.get<json::boolean_t>();
}

//------------------------------------------------------------------------------
// FromJson
//------------------------------------------------------------------------------
opentracing::expected<LightStepTracerOptions> LightStepTracerOptions::FromJson(
    opentracing::string_view options_json) noexcept {
  json parsed_json;
  try {
    parsed_json = json::parse(options_json.data(),
                              options_json.data() + options_json.length());
  } catch (const std::invalid_argument& /*error*/) {
    return opentracing::make_unexpected(std::error_code{});
  }
  json::object_t* object;
  try {
    object = &parsed_json.get_ref<json::object_t&>();
  } catch (const std::domain_error& /*error*/) {
    return opentracing::make_unexpected(std::error_code{});
  }
  LightStepTracerOptions result;
  for (auto i = std::begin(*object); i!= std::end(*object); ++i) {
    if (i->first == "component_name") {
      try {
        result.component_name = std::move(i->second.get_ref<json::string_t&>());
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    } else if (i->first == "access_token") {
      try {
        result.access_token = std::move(i->second.get_ref<json::string_t&>());
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    } else if (i->first == "collector_host") {
      try {
        result.collector_host = std::move(i->second.get_ref<json::string_t&>());
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    } else if (i->first == "collector_port") {
      try {
        result.collector_port = static_cast<uint32_t>(get_integer(i->second));
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    } else if (i->first == "collector_plaintext") {
      try {
        result.collector_plaintext = get_bool(i->second);
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    } else if (i->first == "verbose") {
      try {
        result.verbose = get_bool(i->second);
      } catch (const std::domain_error& /*error*/) {
        return opentracing::make_unexpected(std::error_code{});
      }
    }
  } 
  return opentracing::expected<LightStepTracerOptions>{std::move(result)};
}
} // namespace lightstep

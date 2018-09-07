#include "lightstep_span2.h"

#include <cstdlib>
#include <memory>

namespace lightstep {
const size_t DefaultInitialBlockSize = 512;

//------------------------------------------------------------------------------
// operator new
//------------------------------------------------------------------------------
void* LightStepSpan2::operator new(size_t /*size*/) {
  static_assert(
      DefaultInitialBlockSize > sizeof(LightStepSpan2),
      "Initial block size must be large enough to contain LightStepSpan");
  constexpr auto alignment = alignof(LightStepSpan2);
  void* result;
  auto rcode = posix_memalign(&result, alignment, DefaultInitialBlockSize);
  if (rcode != 0) {
    throw std::bad_alloc{};
  }
  return result;
}

//------------------------------------------------------------------------------
// operator delete
//------------------------------------------------------------------------------
void LightStepSpan2::operator delete(void * ptr) {
  free(ptr);
}

//------------------------------------------------------------------------------
// GetArenaOptions
//------------------------------------------------------------------------------
google::protobuf::ArenaOptions LightStepSpan2::GetArenaOptions() {
  google::protobuf::ArenaOptions result;
  result.initial_block = reinterpret_cast<char*>(this) + sizeof(LightStepSpan2);
  result.initial_block_size = DefaultInitialBlockSize - sizeof(LightStepSpan2);
  return result;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepSpan2::LightStepSpan2(
    std::shared_ptr<const opentracing::Tracer>&& tracer, Logger& logger,
    Recorder& recorder, opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options)
    : arena_{GetArenaOptions()},
      tracer_{std::move(tracer)},
      logger_{logger},
      recorder_{recorder} {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
LightStepSpan2::~LightStepSpan2() {
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void LightStepSpan2::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void LightStepSpan2::SetOperationName(
    opentracing::string_view name) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LightStepSpan2::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan2::SetBaggageItem(
    opentracing::string_view restricted_key,
    opentracing::string_view value) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("SetBaggageItem failed: ", e.what());
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LightStepSpan2::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
} catch (const std::exception& e) {
  logger_.Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void LightStepSpan2::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan2::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
}

//------------------------------------------------------------------------------
// sampled
//------------------------------------------------------------------------------
bool LightStepSpan2::sampled() const noexcept {
  return true;
}
} // namespace lightstep

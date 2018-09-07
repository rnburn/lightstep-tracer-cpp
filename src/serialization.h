#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>

namespace lightstep {
//------------------------------------------------------------------------------
// WireType
//------------------------------------------------------------------------------
// See https://developers.google.com/protocol-buffers/docs/encoding#structure
// Only lists the ones we use
enum class WireType {
  Varint = 0,
  LengthDelimited = 2
};

//------------------------------------------------------------------------------
// StaticKeySerializationSize
//------------------------------------------------------------------------------
template <size_t FieldNumber, WireType WireTypeValue>
struct StaticKeySerializationSize {
  // See https://developers.google.com/protocol-buffers/docs/encoding#structure
  // for documentation on encoding.
  static const size_t value =
      google::protobuf::io::CodedOutputStream::StaticVarintSize32<
          static_cast<uint32_t>((FieldNumber << 3) |
                                static_cast<size_t>(WireTypeValue))>::value;
};

//------------------------------------------------------------------------------
// ComputeSerializationSize
//------------------------------------------------------------------------------
template <size_t FieldNumber>
size_t ComputeSerializationSize(uint64_t x) {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(x);
}

//------------------------------------------------------------------------------
// ComputeSerializationSize
//------------------------------------------------------------------------------
template <size_t FieldNumber>
size_t ComputeSerializationSize(uint32_t x) {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize32(x);
}

//------------------------------------------------------------------------------
// ComputeLengthDelimitedSerializationSize
//------------------------------------------------------------------------------
template <size_t FieldNumber>
size_t ComputeLengthDelimitedSerializationSize(size_t length) {
  return StaticKeySerializationSize<FieldNumber,
                                    WireType::LengthDelimited>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(length) + length;
}
} // namespace lightstep

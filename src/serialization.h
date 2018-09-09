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
// StaticSerializationKey
//------------------------------------------------------------------------------
template <size_t FieldNumber, WireType WireTypeValue>
struct StaticSerializationKey {
  // See https://developers.google.com/protocol-buffers/docs/encoding#structure
  // for documentation on encoding.
  static const uint32_t value = static_cast<uint32_t>(
      (FieldNumber << 3) | static_cast<size_t>(WireTypeValue));
};

//------------------------------------------------------------------------------
// StaticKeySerializationSize
//------------------------------------------------------------------------------
template <size_t FieldNumber, WireType WireTypeValue>
struct StaticKeySerializationSize {
  static const size_t value =
      google::protobuf::io::CodedOutputStream::StaticVarintSize32<
          StaticSerializationKey<FieldNumber, WireTypeValue>::value>::value;
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

//------------------------------------------------------------------------------
// SerializeKeyLength
//------------------------------------------------------------------------------
template <size_t FieldNumber>
void SerializeKeyLength(google::protobuf::io::CodedOutputStream& ostream,
                        size_t length) {
  ostream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::LengthDelimited>::value);
  ostream.WriteVarint64(length);
}

//------------------------------------------------------------------------------
// Serialize
//------------------------------------------------------------------------------
template <size_t FieldNumber>
void SerializeVarint(google::protobuf::io::CodedOutputStream& ostream,
                     uint64_t x) {
  ostream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  ostream.WriteVarint64(x);
}

template <size_t FieldNumber>
void SerializeVarint(google::protobuf::io::CodedOutputStream& ostream,
                     uint32_t x) {
  ostream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  ostream.WriteVarint32(x);
}

template <size_t FieldNumber>
void SerializeString(google::protobuf::io::CodedOutputStream& ostream,
                     opentracing::string_view s) {
  SerializeKeyLength<FieldNumber>(ostream, s.size());
  ostream.WriteRaw(static_cast<const void*>(s.data()), s.size());
}
} // namespace lightstep

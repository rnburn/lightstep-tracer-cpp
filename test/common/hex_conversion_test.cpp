#include "common/hex_conversion.h"

#include <limits>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;
using namespace opentracing;

TEST_CASE("hex-integer conversions") {
  char data[16];
  HexSerializer serializer;

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint32_t x = 0; x < 1000; ++x) {
      {
        REQUIRE(x == *HexToUint64(Uint64ToHex(x, data)));
        auto y = std::numeric_limits<uint64_t>::max() - x;
        REQUIRE(y == *HexToUint64(Uint64ToHex(y, data)));
      }
      {
        REQUIRE(x == *HexToUint64(Uint32ToHex(x, data)));
        auto y = std::numeric_limits<uint32_t>::max() - x;
        REQUIRE(y == *HexToUint64(Uint64ToHex(y, data)));
      }
    }
  }

  SECTION("Verify a few special values.") {
    REQUIRE(Uint64ToHex(0, data) == "0000000000000000");
    REQUIRE(Uint64ToHex(1, data) == "0000000000000001");
    REQUIRE(Uint64ToHex(std::numeric_limits<uint64_t>::max(), data) ==
            "FFFFFFFFFFFFFFFF");

    REQUIRE(*HexToUint64("0") == 0);
    REQUIRE(*HexToUint64("1") == 1);
    REQUIRE(*HexToUint64("FFFFFFFFFFFFFFFF") ==
            std::numeric_limits<uint64_t>::max());
  }

  SECTION("Leading or trailing spaces are ignored when converting from hex.") {
    REQUIRE(*HexToUint64("  \tabc") == 0xabc);
    REQUIRE(*HexToUint64("abc  \t") == 0xabc);
    REQUIRE(*HexToUint64("  \tabc  \t") == 0xabc);
  }

  SECTION("Hex conversion works with both upper and lower case digits.") {
    REQUIRE(*HexToUint64("ABCDEF") == 0xABCDEF);
    REQUIRE(*HexToUint64("abcdef") == 0xABCDEF);
  }

  SECTION("Hex conversion with an empty string gives an error.") {
    REQUIRE(!HexToUint64(""));
    REQUIRE(!HexToUint64("  "));
  }

  SECTION(
      "Hex conversion of a number bigger than "
      "std::numeric_limits<uint64_t>::max() gives an error.") {
    REQUIRE(!HexToUint64("1123456789ABCDEF1"));
  }

  SECTION(
      "Hex conversion of a number within valid limits but with leading zeros "
      "past 16 digits is successful.") {
    REQUIRE(HexToUint64("0123456789ABCDEF1"));
  }

  SECTION("Hex conversion with invalid digits gives an error.") {
    REQUIRE(!HexToUint64("abcHef"));
  }

  SECTION("We can serialize 128-bit integers") {
    REQUIRE(serializer.Uint128ToHex(0, 1) == "0000000000000001");
    REQUIRE(serializer.Uint128ToHex(1, 0) ==
            "0000000000000001"
            "0000000000000000");
    REQUIRE(serializer.Uint128ToHex(std::numeric_limits<uint64_t>::max(),
                                    std::numeric_limits<uint64_t>::max()) ==
            "FFFFFFFFFFFFFFFF"
            "FFFFFFFFFFFFFFFF");
  }
}
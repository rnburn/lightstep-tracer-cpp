#include "common/serialization_chain.h"

#include <iomanip>
#include <random>

#include "test/utility.h"

#include <google/protobuf/io/coded_stream.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SerializationChain") {
  SerializationChain chain;

  std::unique_ptr<google::protobuf::io::CodedOutputStream> stream{
      new google::protobuf::io::CodedOutputStream{&chain}};

  SECTION("An empty chain has no fragments.") {
    stream.reset();
    chain.AddFraming();
    REQUIRE(chain.num_fragments() == 0);
  }

  SECTION("We can write a string smaller than the block size.") {
    stream->WriteString("abc");
    stream.reset();
    chain.AddFraming();
    REQUIRE(chain.num_fragments() == 3);
    REQUIRE(ToString(chain) == AddSpanChunkFraming("abc"));
  }

  SECTION("We can write strings larger than a single block.") {
    std::string s(SerializationChain::FirstBlockSize + 1, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.AddFraming();
    REQUIRE(chain.num_fragments() == 4);
    REQUIRE(ToString(chain) == AddSpanChunkFraming(s));
  }

  SECTION("We can seek to any byte in the fragment stream.") {
    std::string s(SerializationChain::FirstBlockSize + 2, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.AddFraming();
    std::string serialization = AddSpanChunkFraming(s);
    for (size_t i = 1; i <= serialization.size(); ++i) {
      SECTION("cosumption instance " + std::to_string(i)) {
        Consume({&chain}, i);
        REQUIRE(ToString(chain) == serialization.substr(i));
      }
    }
  }

  SECTION("We can advance to any byte in the fragment stream randomly.") {
    std::string s(3 * SerializationChain::FirstBlockSize + 10, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.AddFraming();
    std::string serialization = AddSpanChunkFraming(s);
    std::mt19937 random_number_generator{0};
    for (int i = 0; i < 100; ++i) {
      size_t num_bytes_consumed = 0;
      SECTION("Random advance " + std::to_string(i)) {
        while (num_bytes_consumed < serialization.size()) {
          std::uniform_int_distribution<size_t> distribution{
              1, static_cast<int>(serialization.size()) - num_bytes_consumed};
          auto n = distribution(random_number_generator);
          Consume({&chain}, n);
          num_bytes_consumed += n;
          REQUIRE(ToString(chain) == serialization.substr(num_bytes_consumed));
        }
      }
    }
  }
}

#pragma once

#include <memory>
#include <array>

#include "common/noncopyable.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
class SerializationChain final
    : public google::protobuf::io::ZeroCopyOutputStream,
      private Noncopyable {
  static const int BlockSize = 1024;

  struct Block {
    std::unique_ptr<Block> next;
    std::array<char, BlockSize> data;
  };

 public:
  void AddChunkFraming();

  // ZeroCopyOutputStream
  bool Next(void** data, int* size) override;

  void BackUp(int count) override;

  google::protobuf::int64 ByteCount() const override {
    return static_cast<google::protobuf::int64>(num_bytes_written_);
  }
 private:
   int num_blocks_{0};
   int first_;
   int num_bytes_written_{0};
   int current_block_position_{0};
   Block* current_block_;

   int fragment_index_{0};
   int fragment_position_{0};

   Block head_;
};
} // namespace lightstep

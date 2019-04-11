#include "common/serialization_chain.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// AddChunkFraming
//--------------------------------------------------------------------------------------------------
void SerializationChain::AddChunkFraming() {
}

//--------------------------------------------------------------------------------------------------
// Next
//--------------------------------------------------------------------------------------------------
bool SerializationChain::Next(void** data, int* size) {
  if (current_block_position_ < BlockSize) {
    *size = BlockSize - current_block_position_;
    *data = static_cast<void*>(current_block_->data.data() +
                               current_block_position_);
    num_bytes_written_ += *size;
    current_block_position_ = BlockSize;
    return true;
  }
  current_block_->next.reset(new Block);
  current_block_ = current_block_->next.get();
  current_block_position_ = BlockSize;
  *size = BlockSize;
  num_bytes_written_ += *size;
  *data = static_cast<void*>(current_block_->data.data());
  ++num_blocks_;
  return true;
}

//--------------------------------------------------------------------------------------------------
// BackUp
//--------------------------------------------------------------------------------------------------
void SerializationChain::BackUp(int count) { 
  num_bytes_written_ -= count; 
  current_block_position_ -= count;
}
} // namespace lightstep

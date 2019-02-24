package ringbuf

import (
  "log"
)

type Buffer struct {
  data []byte
  head int
  tail int
  capacity int
}

func NewBuffer(max_size int) *Buffer {
  capacity := max_size + 1
  if capacity <= 0 {
    log.Fatalln("capacity must be greater than zero")
  }
  return &Buffer{
    data: make([]byte, capacity),
    head: 0,
    tail: 0,
    capacity: capacity,
  }
}

func (buffer *Buffer) Size() int {
  if buffer.tail <= buffer.head {
    return buffer.head - buffer.tail
  }
  return buffer.capacity - (buffer.tail - buffer.head)
}

func (buffer *Buffer) Peek() Placement {
  if buffer.tail <= buffer.head {
    return Placement{
      Data1: buffer.data[buffer.tail:buffer.head],
      Data2: nil,
    }
  }
  return Placement{
    Data1: buffer.data[buffer.tail:buffer.capacity],
    Data2: buffer.data[0:buffer.head],
  }
}

func (buffer *Buffer) PeekFree() Placement {
  end := (buffer.capacity + buffer.tail - 1) % buffer.capacity
  if buffer.head <= end {
    return Placement{
      Data1: buffer.data[buffer.head:end],
      Data2: nil,
    }
  }
  return Placement{
    Data1: buffer.data[buffer.head:buffer.capacity],
    Data2: buffer.data[0:end],
  }
}

func (buffer *Buffer) Consume(n int) {
  if (n > buffer.Size()) {
    log.Fatalln("cannot consume more bytes than are in the buffer")
  }
  buffer.tail = (buffer.tail + n) % buffer.capacity
  if buffer.tail == buffer.head {
    buffer.tail = 0
    buffer.head = 0
  }
}

func (buffer *Buffer) Produce(n int) {
  buffer.head = (buffer.head + n) % buffer.capacity
}

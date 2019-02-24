package ringbuf

import (
  "testing"
  "github.com/stretchr/testify/require"
)


func TestEmptyBufferProperties(t *testing.T) {
  buffer := NewBuffer(10)
  require.Equal(t, 0, buffer.Size())  
  space := buffer.Peek()
  require.Equal(t, 0, space.Size())
  free_space := buffer.PeekFree()
  require.Equal(t, 10, free_space.Size())
}

func TestProduceIntoBuffer(t *testing.T) {
  buffer := NewBuffer(10)
  free_space := buffer.PeekFree()
  require.Equal(t, 10, len(free_space.Data1))
  copy(free_space.Data1, "abc")
  buffer.Produce(3)
  contents := buffer.Peek()
  require.Equal(t, 3, contents.Size())
  expected_contents := make([]byte, 3)
  copy(expected_contents, "abc")
  require.Equal(t, expected_contents, contents.Data1)
}

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

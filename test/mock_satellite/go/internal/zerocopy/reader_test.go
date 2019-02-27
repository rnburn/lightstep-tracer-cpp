package zerocopy

import (
  "testing"
  "github.com/stretchr/testify/require"
  "strings"
  "io"
)

func TestRead(t *testing.T) {
  buffer := make([]byte, 10)
  source := io.MultiReader(strings.NewReader("abc"), strings.NewReader("123"))
  reader := NewReader(buffer, source)
  require.Equal(t, 0, len(reader.Bytes()))

  expected_contents := make([]byte, 3)
  copy(expected_contents, "abc")
  require.Nil(t, reader.Read())
  require.Equal(t, expected_contents, reader.Bytes()) 

  expected_contents = make([]byte, 6)
  copy(expected_contents, "abc123")
  require.Nil(t, reader.Read())
  require.Equal(t, expected_contents, reader.Bytes()) 
}

func TestOverflow(t *testing.T) {
  buffer := make([]byte, 5)
  source := io.MultiReader(strings.NewReader("abc"), strings.NewReader("123"))
  reader := NewReader(buffer, source)
  require.Nil(t, reader.Read())
  require.NotNil(t, reader.Read())
}

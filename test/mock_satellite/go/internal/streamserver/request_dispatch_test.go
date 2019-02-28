package streamserver

import (
  "io"
  "strings"
  "testing"
  "github.com/stretchr/testify/require"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopy"
)

func newZeroCopyReader(pieces ... string) *zerocopy.Reader {
  length := 0
  readers := make([]io.Reader, len(pieces))
  for i, s := range pieces {
    length += len(s)
    readers[i] = strings.NewReader(s)
  }
  reader := io.MultiReader(readers...)
  buffer := make([]byte, length + 1)
  return zerocopy.NewReader(buffer, reader)
}

func TestParseRequestLine(t *testing.T) {
  reader := newZeroCopyReader("GET /abc HTTP/1.1\r\n\r\n")
  dispatch, err := parseRequestDispatch(reader)
  require.Nil(t, err)
  require.Equal(t, []byte("GET"), dispatch.method)
  require.Equal(t, []byte("/abc"), dispatch.uri)
}

func TestParseSplitRequestLine(t *testing.T) {
  reader := newZeroCopyReader("GET", " ", "/abc", " ", "HTTP/1.1\r", "\n\r\n")
  dispatch, err := parseRequestDispatch(reader)
  require.Nil(t, err)
  require.Equal(t, []byte("GET"), dispatch.method)
  require.Equal(t, []byte("/abc"), dispatch.uri)
}

func TestParseInvalidRequestLine(t *testing.T) {
  reader := newZeroCopyReader("GET")
  _, err := parseRequestDispatch(reader)
  require.NotNil(t, err)

  reader = newZeroCopyReader("GET /abc")
  _, err = parseRequestDispatch(reader)
  require.NotNil(t, err)

  reader = newZeroCopyReader("GET /abc HTTP/1.1\r")
  _, err = parseRequestDispatch(reader)
  require.NotNil(t, err)
}

func TestParseTransferEncodingHeader(t *testing.T) {
  reader := newZeroCopyReader("GET /abc HTTP/1.1\r\nTransfer-Encoding:chunked\r\n\r\n")
  dispatch, err := parseRequestDispatch(reader)
  require.Nil(t, err)
  require.True(t, dispatch.isChunked)

  reader = newZeroCopyReader("GET /abc HTTP/1.1\r\ntransFer-enCoDing: \tchunked \t\t\r\n\r\n")
  dispatch, err = parseRequestDispatch(reader)
  require.Nil(t, err)
  require.True(t, dispatch.isChunked)
}

func TestParseContentLength(t *testing.T) {
  reader := newZeroCopyReader("GET /abc HTTP/1.1\r\nContent-Length:123\r\n\r\n")
  dispatch, err := parseRequestDispatch(reader)
  require.Nil(t, err)
  require.Equal(t, dispatch.contentLength, 123)
}

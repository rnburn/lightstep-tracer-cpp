package streamserver

import (
  "fmt"
  "bytes"
  "errors"
  "strconv"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopyreader"
)

const (
  parsingRequestLine = 0
  parsingHeader = 1
  httpEndOfLine = "\r\n"
)

type requestDispatch struct {
  method []byte
  uri []byte
  contentLength int
  isChunked bool
  numBytesRead int
}

func newRequestDispatch() *requestDispatch {
  return &requestDispatch{
    contentLength: 0,
    isChunked: false,
    numBytesRead: 0,
  }
}

func (dispatch *requestDispatch) readUntil(reader *zerocopyreader.Reader, delimiter []byte) []byte, error {
  for {
    data := reader.Bytes()[dispatch.numBytesRead:]
    index := bytes.Index(data, delimiter)
    if index >= 0 {
      result := data[:index]
      dispatch.numBytesRead += index + len(delimiter)
      return result, nil
    }
    _, err := reader.Read()
    if err != nil {
      return 0, err
    }
  }
}

func (dispatch *requestDispatch) readRequestLine(reader *zerocopyreader.Reader) error {
  dispatch.method, err := dispatch.readUntil(reader, []byte(" "))
  if err != nil {
    return err
  }
  dispatch.uri, err := dispatch.readUntil(reader, []byte(" "))
  if err != nil {
    return err
  }
  _, err = dispatch.readUntil(reader, []byte(httpEndOfLine))
  return err
}

func (dispatch *requestDispatch) readHeaders(reader *zerocopyreader.Reader) error {
  for {
    headerLine, err := dispatch.readUntil(reader, []byte(httpEndOfLine))
    if err != nil {
      return err
    }
    if len(headerLine) == 0 {
      return nil
    }
    keyIndex := bytes.Index(headerLine, []byte(":"))
    if keyIndex == -1 {
      return errors.New(fmt.Printf("Invalid header line: %s\n", headerLine))
    }
    key := headerLine[:keyIndex]
    value := headerLine[keyIndex+1:]
    if bytes.EqualFold(key, []byte("Transfer-Encoding")) {
      dispatch.isChunked = bytes.Equal(value, []byte("chunked"))
    }
    if bytes.EqualFold(key, []byte("Content-Length")) {
      dispatch.contentLength, err := strconv.Atoi(value)
      if err != nil {
        return errors.New(fmt.Printf("Invalid Content-Length header: %s\n", err.Error()))
      }
    }
  }
}

func readRequestDispatch(reader *zerocopyreader.Reader) error {
  dispatch := newRequestDispatch()
  err := dispatch.readRequestLine(reader)
  if err != nil {
    return err
  }
  return dispatch.readHeaders(reader)
}

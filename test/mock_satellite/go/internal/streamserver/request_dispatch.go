package streamserver

import (
  "fmt"
  "bytes"
  "errors"
  "strconv"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopy"
)

const (
  httpEndOfLine = "\r\n"
)

type requestDispatch struct {
  method []byte
  uri []byte
  contentLength int
  isChunked bool
  numBytesRead int
}

func (dispatch *requestDispatch) readUntil(reader *zerocopy.Reader, delimiter []byte) ([]byte, error) {
  for {
    data := reader.Bytes()[dispatch.numBytesRead:]
    index := bytes.Index(data, delimiter)
    if index >= 0 {
      result := data[:index]
      dispatch.numBytesRead += index + len(delimiter)
      return result, nil
    }
    err := reader.Read()
    if err != nil {
      return nil, err
    }
  }
}

func (dispatch *requestDispatch) parseRequestLine(reader *zerocopy.Reader) error {
  var err error
  dispatch.method, err = dispatch.readUntil(reader, []byte(" "))
  if err != nil {
    return err
  }
  dispatch.uri, err = dispatch.readUntil(reader, []byte(" "))
  if err != nil {
    return err
  }
  _, err = dispatch.readUntil(reader, []byte(httpEndOfLine))
  return err
}

func (dispatch *requestDispatch) parseHeaders(reader *zerocopy.Reader) error {
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
      return errors.New(fmt.Sprintf("Invalid header line: %s\n", headerLine))
    }
    key := headerLine[:keyIndex]
    value := headerLine[keyIndex+1:]
    if bytes.EqualFold(key, []byte("Transfer-Encoding")) {
      dispatch.isChunked = bytes.Equal(value, []byte("chunked"))
    }
    if bytes.EqualFold(key, []byte("Content-Length")) {
      dispatch.contentLength, err = strconv.Atoi(string(value))
      if err != nil {
        return errors.New(fmt.Sprintf("Invalid Content-Length header: %s\n", err.Error()))
      }
      if dispatch.contentLength < 0 {
        return errors.New(fmt.Sprintf("Invalid Content-Length header: %s\n", string(value)))
      }
    }
  }
}

func parseRequestDispatch(reader *zerocopy.Reader) error {
  dispatch := &requestDispatch{
    contentLength: -1,
    isChunked: false,
    numBytesRead: 0,
  }
  err := dispatch.parseRequestLine(reader)
  if err != nil {
    return err
  }
  return dispatch.parseHeaders(reader)
}

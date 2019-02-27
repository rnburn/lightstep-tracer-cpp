package zerocopyreader

import (
  "io"
  "errors"
)

type Reader struct {
  buffer []byte
  reader io.Reader
  numBytesRead int
}

func NewReader(buffer []byte, reader io.Reader) *Reader {
  return &Reader{
    buffer: buffer,
    reader: reader,
    numBytesRead: 0,
  }
}

func (reader *Reader) Bytes() []byte {
  return reader.buffer[:reader.numBytesRead]
}

func (reader *Reader) Read() error {
  n, err := reader.reader.Read(reader.buffer[reader.numBytesRead:])
  reader.numBytesRead += n
  if reader.numBytesRead == len(reader.buffer) {
    return errors.New("Buffer's capacity exceeded")
  }
  return err
}

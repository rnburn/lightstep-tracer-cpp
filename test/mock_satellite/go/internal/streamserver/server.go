package streamserver

import (
  "errors"
  "bytes"
  "log"
  "io"
	"net"
  "sync"
  "sync/atomic"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopy"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/ringbuf"
)

const (
  BufferSize = 1024*1024
)

type Server struct {
  running int32
  mutex sync.Mutex
  listeners map[net.Listener]bool
  connections map[io.Closer]bool
}

func NewServer() *Server {
  return &Server{
    running: 1,
    listeners: make(map[net.Listener]bool),
    connections: make(map[io.Closer]bool),
  }
}

func (server *Server) Serve(listener net.Listener) error {
  server.mutex.Lock()
  if server.listeners == nil {
    server.mutex.Unlock()
    listener.Close()
    return errors.New("Server stopped")
  }
  server.listeners[listener] = true
  server.mutex.Unlock()

  for {
    if atomic.LoadInt32(&server.running) == 0 {
      return nil
    }
    connection, err := listener.Accept()
    if err != nil {
      log.Fatalf("Accept error: %v\n", err)
    }
    go server.handleConnection(connection)
  }
}

func (server *Server) handleConnection(connection net.Conn) {
  server.mutex.Lock()
  if server.connections == nil {
    server.mutex.Unlock()
    connection.Close()
    return
  }
  server.connections[connection] = true
  server.mutex.Unlock()

  defer func() {
    connection.Close()
    server.mutex.Lock()
    defer server.mutex.Unlock()
    // Do I need this?
    if server.connections == nil {
      return
    }
    delete(server.connections, connection) 
  }();

  server.dispatchRequest(connection)
}

func (server *Server) dispatchRequest(connection net.Conn) {
	buffer := ringbuf.NewBuffer(BufferSize)
  reader := zerocopy.NewReader(buffer.PeekFree().Data1, connection) 
  dispatch, err := parseRequestDispatch(reader)
  if err != nil {
    log.Fatalf("Failed to dispatch request: %s\n", err.Error())
  }
  if dispatch.contentLength >= 0 {
    server.handleFixedLengthRequest(io.MultiReader(bytes.NewBuffer(reader.Bytes()), connection))
    return
  }
}

func (server *Server) handleFixedLengthRequest(reader io.Reader) {
  // request, err := http.ReadRequest(bufio.NewReader(reader))
  // if err != nil {
  //   log.Fatalf("Failed tp process fixed length request: %s\n", err.Error())
  // }

}

func (server *Server) Stop() {
  atomic.StoreInt32(&server.running, 0)
  server.mutex.Lock()
  listeners := server.listeners
  server.listeners = nil
  connections := server.connections
  server.connections = nil
  server.mutex.Unlock()

  for listener := range listeners {
    listener.Close()
  }

  for connection := range connections {
    connection.Close()
  }
}

package streamserver

import (
  "errors"
  "log"
  "io"
	"net"
  "sync"
)

type Server struct {
  mutex sync.Mutex
  listeners map[net.Listener]bool
  connections map[io.Closer]bool
}

func NewServer() *Server {
  return &Server{}
}

func (server *Server) Start() error {
  return nil
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
    connection, err := listener.Accept()
    if err != nil {
      log.Fatalf("Accept error: %v\n", err)
    }
    // TODO: check that the server was stopped
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
}

func (server *Server) Stop() {
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

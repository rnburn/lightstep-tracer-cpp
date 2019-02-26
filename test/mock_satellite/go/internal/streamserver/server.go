package streamserver

import (
  "errors"
  "log"
  "io"
	"net"
  "sync"
  "sync/atomic"
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

	defer connection.Close()
	buffer := make([]byte, 512)
	var err error
	for {
		_, err = connection.Read(buffer)
		if err != nil {
			break
		}
	}
	if err != io.EOF {
		log.Fatalf("Read failed: %s\n", err.Error())
	}
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

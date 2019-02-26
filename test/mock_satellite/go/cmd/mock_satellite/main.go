package main

import (
	"fmt"
  "log"
  "strconv"
  "net"
  "os"
  "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/streamserver"
)

func main() {
	if len(os.Args) != 2 {
		log.Fatal("Must provide port")
	}
	port, err := strconv.Atoi(os.Args[1])
	if err != nil {
		log.Fatalf("Failed to parse port: %s\n", err.Error())
	}
	listener, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", port))
	if err != nil {
		log.Fatalf("Listen failed: %s\n ", err.Error())
	}
	log.Printf("Starting mock satellite at %d\n", port)
  server := streamserver.NewServer()
  defer server.Stop()
  err = server.Serve(listener)
  if err != nil {
    log.Fatalf("Failed to run server: %s\n", err.Error())
  }
}

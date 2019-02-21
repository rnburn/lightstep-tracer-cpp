#include "test/mock_satellite/mock_satellite_handle.h"

#include <exception>
#include <iostream>
#include <string>

#include "network/ip_address.h"
#include "network/socket.h"
#include "test/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// IsMockServerUp
//--------------------------------------------------------------------------------------------------
static bool IsMockServerUp(uint16_t port) try {
  Socket socket{};
  socket.SetReuseAddress();
  IpAddress ip_address{"127.0.0.1", port};
  return socket.Connect(ip_address.addr(), sizeof(ip_address.ipv4_address())) ==
         0;
} catch (...) {
  return false;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
MockSatelliteHandle::MockSatelliteHandle(uint16_t port)
    : handle_{"test/mock_satellite/mock_satellite", {std::to_string(port)}} {
  if (!IsEventuallyTrue([port] { return IsMockServerUp(port); })) {
    std::cerr << "Failed to connect to mock satellite\n";
    std::terminate();
  }
}
}  // namespace lightstep
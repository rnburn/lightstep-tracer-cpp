#include "address_info.h"

#include <string>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
AddressInfoList::AddressInfoList(addrinfo* head) noexcept : head_{head} {}

AddressInfoList::AddressInfoList(AddressInfoList&& other) noexcept
    : head_{other.head_} {
  other.head_ = nullptr;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
AddressInfoList::~AddressInfoList() {
  if (head_ != nullptr) {
    freeaddrinfo(head_);
  }
}

//------------------------------------------------------------------------------
// operator=
//------------------------------------------------------------------------------
AddressInfoList& AddressInfoList::operator=(AddressInfoList&& other) &
    noexcept {
  if (head_ != nullptr) {
    freeaddrinfo(head_);
  }
  head_ = other.head_;
  other.head_ = nullptr;
  return *this;
}

//------------------------------------------------------------------------------
// GetAddressInfo
//------------------------------------------------------------------------------
AddressInfoList GetAddressInfo(const char* hostname) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo* head;
  auto rcode = getaddrinfo(hostname, nullptr, &hints, &head);
  if (rcode != 0) {
    throw AddressInfoFailure{gai_strerror(rcode)};
  }
  return AddressInfoList{head};
}

AddressInfoList GetAddressInfo(const char* hostname, uint16_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo* head;
  auto port_str = std::to_string(port);
  auto rcode = getaddrinfo(hostname, port_str.c_str(), &hints, &head);
  if (rcode != 0) {
    throw AddressInfoFailure{gai_strerror(rcode)};
  }
  return AddressInfoList{head};
}
}  // namespace lightstep
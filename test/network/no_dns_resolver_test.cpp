#include "network/dns_resolver.h"

#include "common/logger.h"
#include "network/event_base.h"
#include "test/mock_dns_resolution_callback.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("NoDnsResolver") {
  Logger logger;
  EventBase event_base;
  auto resolver = MakeDnsResolver(logger, event_base, DnsResolverOptions{});
  MockDnsResolutionCallback callback{event_base};

  SECTION("We can resolve raw ip addresses.") {
    resolver->Resolve("192.168.0.2", AF_INET, callback);
    REQUIRE(callback.ipAddresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
    REQUIRE(callback.errorMessage().empty());
  }

  SECTION("Resolving an ipv4 address as ipv6 fails.") {
    resolver->Resolve("192.168.0.2", AF_INET6, callback);
    REQUIRE(callback.ipAddresses().empty());
  }

  SECTION("Resolving a non-ip address fails.") {
    resolver->Resolve("www.google.com", AF_INET, callback);
    REQUIRE(callback.ipAddresses().empty());
  }
}

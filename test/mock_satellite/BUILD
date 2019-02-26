load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_library",
)

lightstep_package()

lightstep_cc_library(
    name = "mock_satellite_lib",
    private_hdrs = [
        "mock_satellite_handle.h",
    ],
    srcs = [
        "mock_satellite_handle.cpp",
    ],
    deps = [
        "//test:child_process_handle_lib",
        "//test:utility_lib",
        "//src/network:socket_lib",
    ],
    data = [
        "//test/mock_satellite/go/cmd/mock_satellite:mock_satellite",
    ],
)

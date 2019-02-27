load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
)

load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

lightstep_package()

go_library(
    name = "go_default_library",
    srcs = [
        "server.go",
        "request_dispatch.go"
    ],
    importpath = "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/streamserver",
    deps = [
        "//test/mock_satellite/go/internal/zerocopyreader:go_default_library",
    ],
)

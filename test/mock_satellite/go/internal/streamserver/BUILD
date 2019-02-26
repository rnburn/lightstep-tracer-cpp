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
    ],
    importpath = "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/streamserver",
)

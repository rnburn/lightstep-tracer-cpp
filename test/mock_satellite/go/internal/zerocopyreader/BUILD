load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
)

load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

lightstep_package()

go_library(
    name = "go_default_library",
    srcs = [
        "reader.go",
    ],
    importpath = "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopyreader",
)

go_test(
    name = "go_default_test",
    srcs = [
        "reader_test.go",
    ],
    deps = [
        "@com_github_stretchr_testify//require:go_default_library",
    ],
    importpath = "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite/go/internal/zerocopyreader",
    embed = [":go_default_library"],
)


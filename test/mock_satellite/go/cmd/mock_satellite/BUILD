load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
)

load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

lightstep_package()

go_binary(
	name = "mock_satellite",
	srcs = [
		"main.go",
	],
  out = "mock_satellite",
  deps = [
      "//test/mock_satellite/go/internal/streamserver:go_default_library",
  ],
)

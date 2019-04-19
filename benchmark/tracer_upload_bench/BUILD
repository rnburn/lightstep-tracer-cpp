load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_binary",
)

lightstep_package()

proto_library(
    name = "tracer_upload_bench_proto",
    srcs = ["tracer_upload_bench.proto"],
    deps = [
        "//lightstep-tracer-configuration:tracer_configuration_proto",
    ],
)

lightstep_cc_binary(
    name = "tracer-upload-bench",
    srcs = glob(["*.h", "*.cpp"]),
    deps = [
        "//:tracer_lib",
        ":tracer_upload_bench_proto_cc",
    ],
    linkopts = [
        "-static-libstdc++",
        "-static-libgcc",
    ],
)

cc_proto_library(
    name = "tracer_upload_bench_proto_cc",
    deps = [":tracer_upload_bench_proto"],
)


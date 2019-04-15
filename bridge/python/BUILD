load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_binary",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_binary(
    name = "lightstep.so",
    linkshared = True,
    srcs = [
        "src/module.cpp",
    ],
    copts = [
        "-DPYTHON_BRIDGE_TRACER_MODULE=\"lightstep\"",
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-Wl,--allow-multiple-definition",
    ],
    deps = [
      "//:tracer_lib",
    ],
    external_deps = [
      "@com_github_rnburn_python_cpp_bridge//src/lib:bridge_tracer_lib",
    ],
    visibility = [
        "//visibility:public",
    ],
)


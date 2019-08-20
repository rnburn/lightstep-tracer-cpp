load("@python_benchmark_pip_deps//:requirements.bzl", "requirement") 
load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
)

lightstep_package()

py_binary(
    name = "span_operations_benchmark",
    srcs = [
        "span_operations_benchmark.py",
    ],
    deps = [
        "//test/mock_satellite:mock_satellite_py_lib",
        requirement("lightstep"),
        requirement("benchmark"),
    ],
    data = [
        "//test/mock_satellite:mock_satellite",
        "//bridge/python/binary/py27mu:lightstep_native.so",
    ],
)

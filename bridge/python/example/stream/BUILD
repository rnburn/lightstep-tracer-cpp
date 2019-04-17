load("@python_bridge_example_deps//:requirements.bzl", "requirement")

py_binary(
    name = "stream",
    srcs = [
        "stream.py",
    ],
    srcs_version = "PY3ONLY",
    default_python_version = "PY3",
    python_version = "PY3",
    data = [
        "//bridge/python:lightstepcpp.so",
    ],
    deps = [
      requirement("opentracing"),
      requirement("basictracer"),
      requirement("lightstep"),
      requirement("protobuf"),
      requirement("thrift"),
      requirement("jsonpickle"),
      requirement("requests"),
      requirement("urllib3"),
      requirement("chardet"),
      requirement("certifi"),
      requirement("googleapis-common-protos"),
    ],
)

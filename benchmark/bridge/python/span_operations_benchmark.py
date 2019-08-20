import lightstep
import benchmark
import sys
import os
from test.mock_satellite.mock_satellite_handle import MockSatelliteHandle

for pyversion in os.listdir('bridge/python/binary'):
    sys.path.append('bridge/python/binary/' + pyversion)
import lightstep_native

PORT = 8000

class BenchmarkBase(benchmark.Benchmark):
    each = 1000

    def setUp(self):
        self.mock_satellite_handle = MockSatelliteHandle(PORT)
        self.size = 100

        self.python_tracer = \
                lightstep.Tracer(
                        component_name = 'benchmark',
                        access_token = 'abc',
                        collector_host = '127.0.0.1',
                        collector_port = PORT,
                        collector_encryption = 'none',
                        use_http = True)
        self.cpp_tracer =  \
                lightstep_native.Tracer(
                        component_name = 'benchmark',
                        access_token='abc',
                        use_stream_recorder=True,
                        collector_plaintext=True,
                        satellite_endpoints=[{'host':'locahost', 'port':PORT}])

    def tearDown(self):
        self.mock_satellite_handle.tearDown()

class Benchmark_SpanCreation(BenchmarkBase):
    def execute(self, tracer):
        for _ in xrange(self.size):
            span = tracer.start_span('abc123')
            span.finish()

    def test_Python(self):
        self.execute(self.python_tracer)

    def test_Cpp(self):
        self.execute(self.cpp_tracer)

class Benchmark_SetTag1(BenchmarkBase):
    def execute(self, tracer):
        for _ in xrange(self.size):
            span = tracer.start_span('abc123')
            span.set_tag('abc0', '123')
            span.finish()

    def test_Python(self):
        self.execute(self.python_tracer)

    def test_Cpp(self):
        self.execute(self.cpp_tracer)

class Benchmark_SetTag2(BenchmarkBase):
    def execute(self, tracer):
        for _ in xrange(self.size):
            span = tracer.start_span('abc123')
            span.set_tag('abc0', '123')
            span.set_tag('abc1', '123')
            span.set_tag('abc2', '123')
            span.set_tag('abc3', '123')
            span.set_tag('abc4', '123')
            span.set_tag('abc5', '123')
            span.set_tag('abc6', '123')
            span.set_tag('abc7', '123')
            span.set_tag('abc8', '123')
            span.set_tag('abc9', '123')
            span.finish()

    def test_Python(self):
        self.execute(self.python_tracer)

    def test_Cpp(self):
        self.execute(self.cpp_tracer)


class Benchmark_Log1(BenchmarkBase):
    def execute(self, tracer):
        for _ in xrange(self.size):
            span = tracer.start_span('abc123')
            span.log_kv({"abc": 123})
            span.finish()

    def test_Python(self):
        self.execute(self.python_tracer)

    def test_Cpp(self):
        self.execute(self.cpp_tracer)

if __name__ == "__main__":
    benchmark.main(format="markdown", numberFormat="%.4g")

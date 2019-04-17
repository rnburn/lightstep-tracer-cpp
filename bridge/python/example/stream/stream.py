import lightstep
import os
import sys
import time
import subprocess
import sys
import json

COLLECTOR_HOST='collector.lightstep.com'
COLLECTOR_PORT=80

def makeSpans(tracer):
    for i in range(1000):
        span = tracer.start_span('span_' + str(i))
        for j in range(25):
            span.set_tag('tag_' + str(j), j)
        span.finish()

def makePythonTracer(access_token):
    return lightstep.Tracer(
            component_name = 'python-native',
            access_token = access_token,
            use_http = True,
            collector_host = COLLECTOR_HOST,
            collector_port = COLLECTOR_PORT,
            collector_encryption = 'none'
    )

def runPythonTracer(access_token):
    tracer = makePythonTracer(access_token)
    t1 = time.time()
    makeSpans(tracer)
    tracer.flush()
    t2 = time.time()
    return t2 - t1

def makeCppTracer(access_token):
    sys.path.append('bridge/python') 
    import lightstepcpp
    return lightstepcpp.make_tracer(json.dumps({
        'component_name': 'python-cpp-stream',
        'access_token': access_token,
        'collector_plaintext': True,
        'use_stream_recorder': True,
        'satellite_endpoints': [
            {'host': COLLECTOR_HOST, 'port': COLLECTOR_PORT}
        ]
    }))

def runCppTracer(access_token):
    tracer = makeCppTracer(access_token)
    t1 = time.time()
    makeSpans(tracer)
    tracer.close()
    t2 = time.time()
    return t2 - t1

if __name__ == "__main__":
    access_token = os.environ['LIGHTSTEP_ACCESS_TOKEN']
    if not access_token:
        print('You must set the environmental variable LIGHTSTEP_ACCESS_TOKEN to your access token')
        os.exit(1)
    if len(sys.argv) != 2:
        print('Usage: stream.py python|cpp')
        sys.exit(1)
    duration = None
    if sys.argv[1] == 'python':
        duration = runPythonTracer(access_token)
    elif sys.argv[1] == 'cpp':
        duration = runCppTracer(access_token)
    else:
        print('Unknown tracer ', sys.argv[1])
        sys.exit(1)
    duration *= 1000
    print('Duration for {} tracer (milliseconds): {}'.format(sys.argv[1], duration))

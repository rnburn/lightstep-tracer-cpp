import lightstep
import os
import time

def makeSpans(tracer):
    for i in range(1000):
        span = tracer.start_span('span_' + str(i))
        for j in range(25):
            span.set_tag('tag_' + str(j), j)
        span.finish()

if __name__ == "__main__":
    access_token = os.environ['LIGHTSTEP_ACCESS_TOKEN']
    if not access_token:
        print('You must set the environmental variable LIGHTSTEP_ACCESS_TOKEN to your access token')
        os.exit(1)
    tracer = lightstep.Tracer(
            component_name = 'test',
            access_token = access_token,
            use_http = True,
            collector_host = 'collector.lightdstep.com',
            collector_port = 80,
            collector_encryption = 'none'
    )

    t1 = time.time()
    makeSpans(tracer)
    t2 = time.time()
    tracer.flush()
    t3 = time.time()
    print('Total duration (milliseconds): ', (t3 - t1)*1000)

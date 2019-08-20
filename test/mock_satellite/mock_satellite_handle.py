from test.child_process_handle import ChildProcessHandle
import socket
import time
import sys

class MockSatelliteHandle:
    def __init__(self, port):
        self.handle = ChildProcessHandle("test/mock_satellite/mock_satellite", [str(port)])
        s = socket.socket()
        start = time.time()
        while True:
            try:
                s.connect(('127.0.0.1', port))
                return
            except:
                now = time.time()
                if now - start > 60:
                    raise RuntimeError('failed to connect to mock satellite')

    def tearDown(self):
        self.handle.tearDown()

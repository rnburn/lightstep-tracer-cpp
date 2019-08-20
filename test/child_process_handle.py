import os
import signal

class ChildProcessHandle:
    def __init__(self, command, args):
        rcode = os.fork()
        if rcode == 0:
            os.execv(command, [command] + args)
        self.pid = rcode

    def tearDown(self):
        os.kill(self.pid, signal.SIGTERM)

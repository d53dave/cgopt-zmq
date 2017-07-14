import os
import asyncio
import zmq.asyncio

import capnp
capnp.remove_import_hook()

THIS_DIR = os.path.dirname(__file__)
tidings_capnp = capnp.load(os.path.join(THIS_DIR, '../capnp/src/tidings.capnp'))
plumbing_capnp = capnp.load(os.path.join(THIS_DIR, '../capnp/src/plumbing.capnp'))

from tornado.ioloop import IOLoop
from tornado.platform.asyncio import AsyncIOMainLoop


# Tell tornado to use asyncio
AsyncIOMainLoop().install()

class RepReqServer:
    def __init__(self, tidings_port, plumbing_port):
        self.tidings_port = tidings_port
        self.plumbing_port = plumbing_port
        self.ctx = zmq.asyncio.Context()
        self.queue = asyncio.Queue()
        self.loop = IOLoop.current()

    async def _plumbing_repreq_loop(self):
        socket = self.ctx.socket(zmq.REP)
        socket.bind('tcp://*:' + str(self.plumbing_port))
        while True:
            greeting = await socket.recv_multipart()
            print('Received' + str(greeting))
            await socket.send_multipart([b"ACK"])

    async def _tidings_repreq_loop(self):
        socket = self.ctx.socket(zmq.REP)
        socket.bind('tcp://*:' + str(self.tidings_port))
        while True:
            greeting = await socket.recv_multipart()
            print('Received' + str(greeting))
            await socket.send_multipart([b"ACK"])

    async def client(self):
        socket = self.ctx.socket(zmq.REQ)
        socket.connect('tcp://127.0.0.1:' + str(self.plumbing_port))
        while True:
            await socket.send_multipart([b"Hello", b"lol"])
            resp = await socket.recv_multipart()
            print(resp)
            await asyncio.sleep(1)

    def _handle_worker_join(self):
        pass

    def _handle_worker_leave(self):
        pass

    def _handle_heartbeat(self):
        pass

    def _handle_stats(self):
        pass

    def start_loop(self):
        """Start the messagequeue loop"""
        self.loop.spawn_callback(self._tidings_repreq_loop)
        self.loop.spawn_callback(self._plumbing_repreq_loop)
        self.loop.spawn_callback(self.client)
        self.loop.start()


if __name__ == '__main__':
    print('Building Server')
    server = RepReqServer(9998, 9999)
    server.start_loop()
    print('Done')

"""Example showing ZMQ with asyncio and tornadoweb integration."""

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

class RepReqServer:
    def __init__(self, tidings_port, plumbing_port):
        self.tidings_port = tidings_port
        self.plumbing_port = plumbing_port

    async def _plumbing_repreq_loop(self, ctx):
        socket = ctx.socket(zmq.REP)
        socket.bind('tcp://*:' + str(port))
        while True:
            greeting = await socket.recv_multipart()
            print('Received' + str(greeting))
            await socket.send_multipart([b"ACK"])

    async def _tidings_repreq_loop(self, ctx):
        socket = ctx.socket(zmq.REP)
        socket.bind('tcp://*:' + str(port))
        while True:
            greeting = await socket.recv_multipart()
            print('Received' + str(greeting))
            await socket.send_multipart([b"ACK"])

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
         # Tell tornado to use asyncio
        AsyncIOMainLoop().install()

        # This must be instantiated after the installing the IOLoop
        queue = asyncio.Queue()
        ctx = zmq.asyncio.Context()   
        loop = IOLoop.current()

        loop.spawn_callback(lambda: self._plumbing_repreq_loop(ctx, self.plumbing_port))
        loop.spawn_callback(lambda: self._tidings_repreq_loop(ctx, self.tidings_port))
        loop.start()


async def client(ctx, port):
    socket = ctx.socket(zmq.REQ)
    socket.connect('tcp://127.0.0.1:' + str(port))
    while True:
        await socket.send_multipart([b"Hello", b"lol"])
        resp = await socket.recv_multipart()
        print(resp)
        await asyncio.sleep(1)



if __name__ == '__main__':
    server = RepReqServer(9998, 9999)
    
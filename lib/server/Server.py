"""Example showing ZMQ with asyncio and tornadoweb integration."""

import os
import asyncio
import zmq.asyncio

import capnp
capnp.remove_import_hook()
this_dir = os.path.dirname(__file__)
tidings_capnp = capnp.load(os.path.join(this_dir, '../capnp/src/tidings.capnp'))

from tornado.ioloop import IOLoop
from tornado.platform.asyncio import AsyncIOMainLoop

async def server(ctx, port):
    socket = ctx.socket(zmq.REP)
    socket.bind('tcp://*:' + str(port))
    while True:
        greeting = await socket.recv_multipart()
        print('Received' + str(greeting))
        await socket.send_multipart([b"ACK"])

async def client(ctx, port):
    socket = ctx.socket(zmq.REQ)
    socket.connect('tcp://127.0.0.1:' + str(port))
    while True:
        await socket.send_multipart([b"Hello", b"lol"])
        resp = await socket.recv_multipart()
        print(resp)
        await asyncio.sleep(1)


def zmq_tornado_loop(port):
    """Start the messagequeue loop"""
     # Tell tornado to use asyncio
    AsyncIOMainLoop().install()

    # This must be instantiated after the installing the IOLoop
    queue = asyncio.Queue()
    ctx = zmq.asyncio.Context()   
    loop = IOLoop.current()

    loop.spawn_callback(lambda: server(ctx, port))
    loop.spawn_callback(lambda: client(ctx, port))
    loop.start()

if __name__ == '__main__':
    zmq_tornado_loop(9988)

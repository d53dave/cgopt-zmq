"""Test suite for the plumbing part of the message queue"""

import pytest
import os
import arrow
import zmq
from context import messagequeue
from tornado.ioloop import IOLoop
import capnp
capnp.remove_import_hook()

CWD = os.path.dirname(__file__)
plumbing_capnp = capnp.load(
    os.path.join(CWD, '../messagequeue/capnp/src/plumbing.capnp'))

TESTPORT = 9988


class QueueClient:
    def __init__(self):
        self.ctx = zmq.asyncio.Context()
        self.socket = self.ctx.socket(zmq.REQ)

    async def send_request(self, message):
        self.socket.connect('tcp://127.0.0.1:' + str(TESTPORT))
        await self.socket.send_multipart(message)
        return await self.socket.recv_multipart()


@pytest.fixture
def csaopt_server():
    '''Returns an instance of the messagequeue server'''
    server = messagequeue.RepReqServer(IOLoop.current(), TESTPORT+1, TESTPORT)
    yield server
    server.stop()
    # return None


@pytest.fixture
def csaopt_client():
    '''Returns a client instance'''
    yield QueueClient()
    print("Client cleanup")
    

def test_join(csaopt_server, csaopt_client):
    """A worker wants to join the hive"""
    with csaopt_server:
        print('Test Join entered')
        message = plumbing_capnp.Plumbing.new_message(
            id='1234', 
            sender='worker1',
            timestamp=arrow.utcnow().timestamp,
            type='register'
        )

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: csaopt_client.send_request([message.to_bytes_packed()]))

        message = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert message.type == 'ack'

# def TestLeaveAfterJoin():
#     """A worker wants to leave after it joined"""
#     pass


# def TestDoubleJoin():
#     """A worker tries to join after it already joined"""
#     pass


# def TestLeaveWithoutJoin():
#     """A worker tries to leave before it joined"""
#     pass

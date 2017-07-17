"""Test suite for the plumbing part of the message queue"""

import pytest
import os
import arrow
import zmq
from server import messagequeue
import capnp
capnp.remove_import_hook()

THIS_DIR = os.path.dirname(__file__)
plumbing_capnp = capnp.load(
    os.path.join(THIS_DIR, '../../lib/capnp/src/plumbing.capnp'))

PLUMBING_PORT = 9988


class QueueClient:
    def __init__(self):
        self.ctx = zmq.Context()
        self.socket = self.ctx.socket(zmq.REQ)

    def send_request(self, message):
        self.socket.connect('tcp://127.0.0.1:' + str(PLUMBING_PORT))
        self.socket.send(message)
        return self.socket.recv()


@pytest.fixture
def csaopt_server():
    '''Returns an instance of the messagequeue server'''
    return messagequeue.RepReqServer(PLUMBING_PORT, PLUMBING_PORT + 1)


@pytest.fixture
def my_client():
    '''Returns a client instance'''
    return QueueClient()


def test_join(csaopt_server, my_client):
    """A worker wants to join the hive"""
    message = plumbing_capnp.Plumbing.new_message()
    message.id = '1234'
    message.sender = 'worker1'
    message.timestamp = arrow.utcnow().timestamp
    message.type = 'register'

    csaopt_server.start_loop()
    response = my_client.send_request(message.to_bytes())

    assert response.type == 'ack'

# def TestLeaveAfterJoin():
#     """A worker wants to leave after it joined"""
#     pass


# def TestDoubleJoin():
#     """A worker tries to join after it already joined"""
#     pass


# def TestLeaveWithoutJoin():
#     """A worker tries to leave before it joined"""
#     pass

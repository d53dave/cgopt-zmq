"""Test suite for the plumbing part of the message queue"""

import pytest
import os
import arrow
import zmq
from context import messagequeue
from tornado.ioloop import IOLoop
import capnp
import asyncio
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
    """A worker wants to register with the messagequeue"""
    with csaopt_server:
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


def test_double_register(csaopt_server, csaopt_client):
    """A worker tries to register after it already registered"""
    with csaopt_server:
        message = plumbing_capnp.Plumbing.new_message(
            id='1235',
            sender='worker2',
            timestamp=arrow.utcnow().timestamp,
            type='register'
        )

        async def doublerequest(message):
            bytes = message.to_bytes_packed()
            await csaopt_client.send_request([bytes])
            return await csaopt_client.send_request([bytes])

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: doublerequest(message))

        response = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert response.type == 'error'
        assert 'already registered' in str(response.message)
        assert str(message.sender) in str(response.message)


def test_unregister_after_register(csaopt_server, csaopt_client):
    """A worker tries to leave after it joined"""
    with csaopt_server:
        join = plumbing_capnp.Plumbing.new_message(
            id='1237',
            sender='worker3',
            timestamp=arrow.utcnow().timestamp,
            type='register'
        )
        leave = plumbing_capnp.Plumbing.new_message(
            id='1238',
            sender='worker3',
            timestamp=arrow.utcnow().timestamp,
            type='unregister'
        )

        async def join_and_leave(joinmessage, leavemessage):
            await csaopt_client.send_request([joinmessage.to_bytes_packed()])
            return await csaopt_client.send_request([leavemessage.to_bytes_packed()])

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: join_and_leave(join, leave))

        response = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert response.type == 'ack'
        assert response.id == '1238'


def test_unregister_without_register(csaopt_server, csaopt_client):
    """A worker wants to unregister with the messagequeue but was not registered"""
    with csaopt_server:
        message = plumbing_capnp.Plumbing.new_message(
            id='1234', 
            sender='worker1',
            timestamp=arrow.utcnow().timestamp,
            type='unregister'
        )

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: csaopt_client.send_request([message.to_bytes_packed()]))

        message = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert message.type == 'error'
        assert message.sender in str(message.message)
        assert 'not previously registered' in str(message.message)


def test_hearbeat(csaopt_server, csaopt_client):
    """A worker wants to register with the messagequeue"""
    with csaopt_server:
        join = plumbing_capnp.Plumbing.new_message(
            id='1234',
            sender='worker9',
            timestamp=arrow.utcnow().timestamp,
            type='register'
        )

        heartbeat = plumbing_capnp.Plumbing.new_message(
            sender='worker9',
            type='heartbeat'
        )

        async def join_and_heartbeat(joinmessage, heartbeatmessage):
            await csaopt_client.send_request([joinmessage.to_bytes_packed()])
            return await csaopt_client.send_request([heartbeatmessage.to_bytes_packed()])

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: join_and_heartbeat(join, heartbeat))

        message = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert message.type == 'ack'


def test_sad_heartbeat(csaopt_server, csaopt_client):
    """A worker wants to register with the messagequeue"""
    with csaopt_server:
        message = plumbing_capnp.Plumbing.new_message(
            id='12342',
            sender='worker1',
            timestamp=arrow.utcnow().timestamp,
            type='heartbeat'
        )

        ioloop = IOLoop.current()
        response = ioloop.run_sync(
            lambda: csaopt_client.send_request([message.to_bytes_packed()]))

        message = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert message.type == 'error'
        assert 'not known' in str(message.message)


@pytest.mark.timeout(10)
def test_timeout(csaopt_server, csaopt_client):
    """A worker wants to send a heartbeat, but it already timed out"""
    with csaopt_server:
        csaopt_server.timeout = 1.0

        join = plumbing_capnp.Plumbing.new_message(
            id='1234',
            sender='worker7',
            timestamp=arrow.utcnow().timestamp,
            type='register'
        )

        heartbeat = plumbing_capnp.Plumbing.new_message(
            sender='worker7',
            type='heartbeat'
        )

        async def join_and_heartbeat(joinmessage, heartbeatmessage):
            await csaopt_client.send_request([joinmessage.to_bytes_packed()])
            await asyncio.sleep(1.5)
            return await csaopt_client.send_request([heartbeatmessage.to_bytes_packed()])

        ioloop = IOLoop.current()
        response = ioloop.run_sync(lambda: join_and_heartbeat(join, heartbeat))

        message = plumbing_capnp.Plumbing.from_bytes_packed(response[0])
        assert message.type == 'error'
        assert 'not known' in str(message.message)

import os
import asyncio
import zmq.asyncio
import logging
import arrow
from .strings import messages
from tornado.ioloop import IOLoop
from tornado.platform.asyncio import AsyncIOMainLoop
import capnp

AsyncIOMainLoop().install()

capnp.remove_import_hook()

logger = logging.getLogger(__name__)

CWD = os.path.dirname(__file__)
tidings_capnp = capnp.load(os.path.join(CWD, 'capnp/src/tidings.capnp'))
plumbing_capnp = capnp.load(os.path.join(CWD, 'capnp/src/plumbing.capnp'))

# Tell tornado to use asyncio


class RepReqServer:
    def __init__(self, ioloop=IOLoop.current(), tidings_port=9988, plumbing_port=9989):
        self.tidings_port = tidings_port
        self.plumbing_port = plumbing_port
        self.ctx = zmq.asyncio.Context()
        self.queue = asyncio.Queue()
        self.run = True
        self.plumbingsocket = None
        self.tidingsocket = None

        self.workers = {}


        ioloop.spawn_callback(self._tidings_repreq_loop)
        ioloop.spawn_callback(self._plumbing_repreq_loop)

    async def _plumbing_repreq_loop(self):
        self.plumbingsocket = self.ctx.socket(zmq.REP)

        self.plumbingsocket.bind('tcp://*:' + str(self.plumbing_port))
        while self.run:
            try:
                packed_bytes = await asyncio.wait_for(self.plumbingsocket.recv_multipart(), timeout=0.5)
                logger.debug('Received packed bytes with length {}'.format(len(packed_bytes)))
                message = plumbing_capnp.Plumbing.from_bytes_packed(packed_bytes[0])

                response = None

                if message.type == 'register':  # comparison with 'is' returns false
                    response = self._handle_worker_register(message)
                elif message.type == 'unregister':
                    response = self._handle_worker_unregister(message)
                else:
                    raise NotImplementedError
                
                response_bytes = response.to_bytes_packed()

                logger.debug('Sending response {} with length {}'.format(response, len(response_bytes)))
                await self.plumbingsocket.send_multipart([response_bytes], flags=zmq.NOBLOCK)
            except asyncio.TimeoutError:
                logger.info('Timeout')
                continue

        self.plumbingsocket.close(linger=0.0)

    async def _tidings_repreq_loop(self):
        self.tidingsocket = self.ctx.socket(zmq.REP)

        self.tidingsocket.bind('tcp://*:' + str(self.tidings_port))
        while self.run:
            greeting = await asyncio.wait_for(self.tidingsocket.recv_multipart(), timeout=0.5)
            print('Received' + str(greeting))
            await self.tidingsocket.send_multipart([b"ACK"], flags=zmq.NOBLOCK)
        
        self.tidingsocket.close(linger=0.0)

    def _handle_worker_register(self, request: plumbing_capnp.Plumbing) -> plumbing_capnp.Plumbing:
        response = plumbing_capnp.Plumbing.new_message()
        now = arrow.utcnow()

        response.id = request.id
        response.timestamp = now.timestamp

        if request.sender in self.workers:
            response.type = 'error'
            response.message = messages['ALREADY_REGISTERED'].format(request.sender)
        else:
            self.workers[request.sender] = now
            response.type = 'ack'
        
        return response

    def _handle_worker_unregister(self, request):
        response = plumbing_capnp.Plumbing.new_message()
        now = arrow.utcnow()

        response.id = request.id
        response.timestamp = now.timestamp
        
        if request.sender in self.workers:
            del self.workers[request.sender]
            response.type = 'ack'
        else:
            response.type = 'error'
            response.message = messages['NOT_REGISTERED'].format(request.sender)


        return response

    def _handle_heartbeat(self):
        pass

    def _handle_stats(self):
        pass

    def stop(self):
        self.run = False

    def __enter__(self):
        pass

    def __exit__(self, type, value, tb):
        if self.tidingsocket:
            self.tidingsocket.close()
        if self.plumbingsocket:
            self.plumbingsocket.close()

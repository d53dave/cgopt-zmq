#include <zmqpp/zmqpp.hpp>
#include <zmq.hpp>│
#include <string>
#include <iostream>
#include <queue>
#include "server.hpp"

using namespace std;

int main(int argc, char *argv[]) {
	serve_pubsub(5555);
}

void serve_pubsub(int port) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	socket.bind(("tcp://*:"+to_string(port)).c_str());

	while (true) {
		zmq::message_t request;

		//  Wait for next request from client
		socket.recv(&request);


		//  Send reply back to client
		zmq::message_t reply(5);
		memcpy((void *) reply.data(), "World", 5);
		socket.send(reply);
	}
}

void process_pp_msg(zmq::message_t &req, zmq::message_t &res) {

}

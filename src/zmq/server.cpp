#include <zmqpp/zmqpp.hpp>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <queue>
#include "server.hpp"
#include "spdlog.h"
#include "cgopt.pb.h"

using namespace std;

auto console = spdlog::stdout_logger_mt("console");

void serve_pubsub(string host, int port);

int main(int argc, char *argv[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	serve_pubsub("*", 5555);
}

void serve_pubsub(string host, int port) {
	zmqpp::context context;
	zmqpp::socket_type type = zmqpp::socket_type::pull;
	zmqpp::socket socket (context, type);
    console->info() << "Binding to "<<host<<":"<<port; 
	socket.bind(("tcp://"+host+":"+to_string(port)).c_str());

	queue<cgopt_response> worker_queue;
	queue<cgopt_response> server_queue;

	size_t processed = 0;
	auto start = std::chrono::high_resolution_clock::now();

	while (true) {
		zmqpp::message message;
		cgopt_request request;

		//  Wait for next request from client
		socket.receive(message);
		string serialized_req;

		message >> serialized_req;

		console->info() << "Parsing successful: "<<request.ParseFromString(serialized_req);
		console->info() << request.SerializeAsString();

		if(request.type() == request.GET_WORK){
			if(worker_queue.empty()){

			} else {
				zmqpp::message respmessage;
				respmessage << worker_queue.front().SerializeAsString();
				socket.send(respmessage);
				worker_queue.pop();
			}
		} else if (request.type() == request.PUT_WORK) {
			cgopt_response response;
			response.set_type(response.WORK);
			response.set_id(request.id());
			for(int i=0; i<request.data_size();++i){
				response.set_data(i, request.data(i));
			}
			worker_queue.push(response);
		} else {
			console->warn() << "Request message type not recognized";
		}
		++processed;
		if(processed % 10001 == 0){
			auto now = chrono::high_resolution_clock::now();
			cout << "Processed " << processed << " messages in "
						  << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
						  << " milliseconds\n";
			start = std::chrono::high_resolution_clock::now();
		}

	}
}

void process_pp_msg(zmq::message_t &req, zmq::message_t &res) {

}

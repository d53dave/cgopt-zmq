#include <zmqpp/zmqpp.hpp>
#include <zmq.h>
#include <string>
#include <iostream>
#include <queue>
#include "server.hpp"
#include <spdlog/spdlog.h>
#include <tidings/tiding.capnp.h>

auto console = spdlog::stdout_logger_mt("console");

//void debug_request(cgopt_request request);
//void debug_response(cgopt_response response);
void serve_pubsub(std::string host, int port);
//cgopt_response process_req(cgopt_request request,
//		std::queue<cgopt_response> & worker_queue, std::queue<cgopt_response> & server_queue);

int main(int argc, char *argv[]) {

//	GOOGLE_PROTOBUF_VERIFY_VERSION;

	serve_pubsub("*", 5555);
}

void serve_pubsub(std::string host, int port) {
	zmqpp::context context;
	zmqpp::socket_type type = zmqpp::socket_type::rep;
	zmqpp::socket socket(context, type);
	console->info() << "Binding to " << host << ":" << port;
	socket.bind(("tcp://" + host + ":" + std::to_string(port)).c_str());

//	std::queue<cgopt_response> worker_queue;
//	std::queue<cgopt_response> server_queue;

	size_t processed = 0;
	auto start = std::chrono::high_resolution_clock::now();

	for(int i = 0; i < 100; ++i) {
		zmqpp::message reqmessage;
//		cgopt_request request;

		//  Wait for next request from client
		socket.receive(reqmessage);
		std::string serialized_req;

		reqmessage >> serialized_req;
//		request.ParseFromString(serialized_req);

//		debug_request(request);

		//get response
//		cgopt_response response = process_req(request, worker_queue,
//				server_queue);

//		debug_response(response);

		//and send it
		zmqpp::message respmessage;
//		respmessage << response.SerializeAsString();
		socket.send(respmessage);

		++processed;
		if (processed % 10001 == 0) {
			auto now = std::chrono::high_resolution_clock::now();
			std::cout << "Processed " << processed << " messages in "
					<< std::chrono::duration_cast<std::chrono::milliseconds>(
							now - start).count() << " milliseconds\n";
			start = std::chrono::high_resolution_clock::now();
		}

	}
}

//cgopt_response process_req(cgopt_request request,
//		std::queue<cgopt_response> &  worker_queue,
//		std::queue<cgopt_response> &  server_queue) {
//
//	cgopt_response response;
//	response.set_id(request.id());
//
//	if (request.type() == request.GET_WORK) {
//		response.set_type(response.WORK);
//		if (worker_queue.empty()) {
//			response.set_error(NO_WORK);
//		} else {
//			response.CopyFrom(worker_queue.front());
//			worker_queue.pop();
//			console->info() << "POP Queue size="<<worker_queue.size();
//		}
//	} else if (request.type() == request.PUT_WORK) {
//		cgopt_response queue_response;
//		queue_response.set_type(response.WORK);
//		queue_response.set_id(request.id());
//		queue_response.mutable_data()->CopyFrom(request.data());
////		memcpy(queue_response.mutable_data()->mutable_data(),
////				response.data().data(), sizeof(double)*response.data_size());
//		worker_queue.push(queue_response);
//		console->info() << "PUSH Queue size="<<worker_queue.size();
//		response.set_type(response.OK);
//	} else {
//		console->warn() << "Request message type not recognized";
//		response.set_error(BAD_REQUEST);
//	}
//	return response;
//}
//
//
//void debug_request(cgopt_request request) {
//	std::string s;
//	google::protobuf::TextFormat::PrintToString(request, &s);
//	console->info() << "Request " << s;
//}
//
//void debug_response(cgopt_response response) {
//
//	std::string s;
//	google::protobuf::TextFormat::PrintToString(response, &s);
//	console->info() << "Response " << s;
//}

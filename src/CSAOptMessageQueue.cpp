#include "CSAOptMessageQueue.h"
#include <stdio.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>
#include <tidings/tidings.capnp.h>
#include <tidings/plumbing.capnp.h>
#include <thread>

CSAOpt::MessageQueue::MessageQueue(int tidingsPort, int plumbingPort) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
    this->logger = std::make_shared<spdlog::logger>("csaopt-zmq-logger", begin(sinks), end(sinks));
    spdlog::register_logger(this->logger);

    std::string host{"*"};


    zmqpp::context context2;
    zmqpp::socket_type type = zmqpp::socket_type::rep;


    zmqpp::socket socket2(context2, type);


    socket2.bind(("tcp://" + host + ":" + std::to_string(plumbingPort)).c_str());


    this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

    MessageQueue::run = true;

    std::thread plumbingRepReqThread([=] { runPlumbingRepReqLoop(host, plumbingPort); } );
    std::thread tidingsRepReqThread([=] { runTidingsRepReqLoop(host, tidingsPort); } );

    tidingsRepReqThread.join();
    plumbingRepReqThread.join();
};

void CSAOpt::MessageQueue::runTidingsRepReqLoop(std::string host, unsigned int port) {
    this->logger->info("Entering Tiding REP/REQ loop");

    zmqpp::context context;
    zmqpp::socket_type type = zmqpp::socket_type::rep;
    zmqpp::socket socket(context, type);

    std::string addr = "tcp://" + host + ":" + std::to_string(port);
    this->logger->info("Binding socket on {}", addr);
    socket.bind(addr.c_str());

    while(this->run){
        zmqpp::message reqmessage;

        socket.receive(reqmessage);

        const std::FILE* tmpf = std::tmpfile();
        reqmessage >> tmpf;

        ::capnp::PackedFdMessageReader capnpMessage(fileno(const_cast<std::FILE*>(tmpf)));
    }
}

void CSAOpt::MessageQueue::runPlumbingRepReqLoop(std::string host, unsigned int port) {
    this->logger->info("Entering Plumbing REP/REQ loop");

    zmqpp::context context;
    zmqpp::socket_type type = zmqpp::socket_type::rep;
    zmqpp::socket socket(context, type);

    std::string addr = "tcp://" + host + ":" + std::to_string(port);
    this->logger->info("Binding socket on {}", addr);
    socket.bind(addr.c_str());

    while(this->run){
        zmqpp::message reqmessage;

        socket.receive(reqmessage);

        const std::FILE* tmpf = std::tmpfile();
        reqmessage >> tmpf;

        ::capnp::PackedFdMessageReader capnpMessage(fileno(const_cast<std::FILE*>(tmpf)));
    }
}

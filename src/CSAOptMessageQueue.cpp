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
    this->run = true;

    this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

    std::thread plumbingRepReqThread([=] { runPlumbingRepReqLoop(host, plumbingPort); } );
    std::thread tidingsRepReqThread([=] { runTidingsRepReqLoop(host, tidingsPort); } );

    tidingsRepReqThread.join();
    plumbingRepReqThread.join();

    this->logger->info("Threads exited. Terminating.");
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
        const std::FILE* tmpf = std::tmpfile();
        this->readMessageToTmpFile(socket, tmpf);

        ::capnp::PackedFdMessageReader capnpMessage(fileno(const_cast<std::FILE*>(tmpf)));

        Tidings::Reader tidings = capnpMessage.getRoot<Tidings>();

        this->logger->info("Received tiding with Id {}", tidings.getId().cStr());
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
        const std::FILE* tmpf = std::tmpfile();
        this->readMessageToTmpFile(socket, tmpf);

        ::capnp::PackedFdMessageReader capnpMessage(fileno(const_cast<std::FILE*>(tmpf)));

        Plumbing::Reader tidings = capnpMessage.getRoot<Plumbing>();

        this->logger->info("Received plumbing with Id {}", tidings.getId().cStr());
    }
}

void CSAOpt::MessageQueue::readMessageToTmpFile(zmqpp::socket& socket, const std::FILE *file) const{
    zmqpp::message reqmessage;

    socket.receive(reqmessage);
    reqmessage >> file;
}

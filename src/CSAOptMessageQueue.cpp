#include "CSAOptMessageQueue.h"
#include <stdio.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>
#include <tidings/tidings.capnp.h>
#include <tidings/plumbing.capnp.h>
#include <thread>
#include <future>
#include <sstream>
#include "kj/KjStringPipe.h"

void write_to_the_pipe(int fd, std::string data)
{
    FILE *stream;
    stream = fdopen(fd, "w");
    for(char& c : data) {
        fprintf(stream, "%c", c);
    }
    fclose (stream);
}

CSAOpt::MessageQueue::MessageQueue(int tidingsPort, int plumbingPort) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
    this->logger = std::make_shared<spdlog::logger>("csaopt-zmq-logger", begin(sinks), end(sinks));
    spdlog::register_logger(this->logger);

    std::string host{"*"};
    this->run = true;

    this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

    this->plumbingRepReqThread = std::thread([=] { runPlumbingRepReqLoop(host, plumbingPort); } );
    this->tidingsRepReqThread = std::thread([=] { runTidingsRepReqLoop(host, tidingsPort); } );

    this->logger->info("Started threads {} and {}", this->plumbingRepReqThread.get_id(), this->tidingsRepReqThread.get_id());
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
        logger->info("Plumbing rep/req loop pass");
        zmqpp::message req;
        socket.receive(req);

        logger->info("Plumbing rep/req loop recv message");
        std::string strBuf;
        req >> strBuf;

        kj::std::StringPipe pipe(strBuf);
        ::capnp::PackedMessageReader recvMessage(pipe);

        logger->info("Plumbing rep/req loop message deserialized");

        Plumbing::Reader recvPlumbing = recvMessage.getRoot<Plumbing>();

        this->logger->info("Received plumbing with Id {}", recvPlumbing.getId().cStr());

        ::capnp::MallocMessageBuilder message;
        Plumbing::Builder plumbing = message.initRoot<Plumbing>();
        std::time_t t = std::time(0);
        plumbing.setId(recvMessage.getRoot<Plumbing>().getId().cStr());
        plumbing.setTimestamp(t);
        plumbing.setSender("zmq");
        plumbing.setType(Plumbing::Type::ACK);

        pipe.clear();
        writePackedMessage(pipe, message);

        zmqpp::message resp;
        resp << pipe.getData();

        socket.send(resp);
    }
}

CSAOpt::MessageQueue::~MessageQueue(){
    this->logger->info("Destructor for MessageQueue");
    this->run = false;

    std::future<void> future = std::async ([=]{
        this->logger->info("Destructor waiting for threads to join");
        this->plumbingRepReqThread.join();
        this->tidingsRepReqThread.join();
    });

    std::chrono::seconds myTimeout(2);
    if(future.wait_for(myTimeout) == std::future_status::timeout){
        // TODO: Lol, this is obviously bad.
        std::terminate();
    }

    this->logger->info("Destructor for MessageQueue done.");
}

void CSAOpt::MessageQueue::readMessageToTmpFile(zmqpp::socket& socket, const std::FILE *file) const{
    zmqpp::message reqmessage;

    socket.receive(reqmessage);
    reqmessage >> file;
}

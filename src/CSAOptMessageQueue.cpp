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
#include <set>
#include "kj/KjStringPipe.h"
#include <iterator>

CSAOpt::MessageQueue::MessageQueue(int tidingsPort, int plumbingPort) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
    this->logger = std::make_shared<spdlog::logger>("csaopt-zmq-logger", begin(sinks), end(sinks));
    spdlog::register_logger(this->logger);

    std::string host{"*"};
    this->run = true;

    this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

    this->plumbingRepReqThread = std::thread([=] { runPlumbingRepReqLoop(host, plumbingPort); } );
    this->tidingsRepReqThread = std::thread([=] { runTidingsRepReqLoop(host, tidingsPort); } );

    this->logger->info("Started threads REQ/REP threads");
};

void CSAOpt::MessageQueue::runTidingsRepReqLoop(std::string host, unsigned int port) {
    this->logger->info("Entering Tiding REP/REQ loop");

    zmqpp::context context;
    zmqpp::socket_type type = zmqpp::socket_type::rep;
    zmqpp::socket socket(context, type);

    std::string addr = "tcp://" + host + ":" + std::to_string(port);
    this->logger->info("Binding socket on {}", addr);
    socket.bind(addr.c_str());

    zmqpp::poller poller;
    poller.add(socket);

    while(this->run){
        if(poller.poll(100)) {

        }
    }
}

void CSAOpt::MessageQueue::runPlumbingRepReqLoop(std::string host, unsigned int port) {
    this->logger->info("Entering Plumbing REP/REQ loop");

    std::set<workerId> workers;

    zmqpp::context context;
    zmqpp::socket_type type = zmqpp::socket_type::rep;
    zmqpp::socket socket(context, type);

    std::string addr = "tcp://" + host + ":" + std::to_string(port);
    this->logger->info("Binding socket on {}", addr);
    socket.bind(addr.c_str());

    zmqpp::poller poller;
    poller.add(socket);

    while(this->run){
        if(poller.poll(100)) {
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

            switch (recvPlumbing.getType()) {
                case Plumbing::Type::REGISTER:
                    handleRegister(plumbing, workers, recvPlumbing);
                    break;
                case Plumbing::Type::UNREGISTER:
                    handleUnregister(plumbing, workers, recvPlumbing);
                    break;
                case Plumbing::Type::HEARTBEAT:
                    break;
                case Plumbing::Type::STATS:
                    break;
                default:
                    break;
            }

            pipe.clear();
            writePackedMessage(pipe, message);

            logger->info("Sending response for id {}", plumbing.getId().cStr());

            zmqpp::message resp;
            resp << pipe.getData();

            socket.send(resp);
        }
    }
}
void CSAOpt::MessageQueue::handleUnregister(Plumbing::Builder& response, std::set<workerId>& set, Plumbing::Reader& recvmessage) {
    response.setId(recvmessage.getId());
    response.setTimestamp(time_t(0));
    std::set<std::string>::iterator it = set.find(std::string{recvmessage.getSender().cStr()});
    if(it == set.end()){
        response.setType(Plumbing::Type::ERROR);
        response.setMessage("Worker '"+std::string{recvmessage.getSender().cStr()}+"' is not registered");
    } else {
        set.erase(it);
        response.setType(Plumbing::Type::ACK);
    }
}


void CSAOpt::MessageQueue::handleRegister(Plumbing::Builder& response, std::set<workerId>& set, Plumbing::Reader& recvmessage) {
    response.setId(recvmessage.getId());
    response.setTimestamp(time_t(0));
    if(set.count(recvmessage.getSender().cStr()) > 0){
        response.setType(Plumbing::Type::ERROR);
        response.setMessage("Worker '"+std::string{recvmessage.getSender().cStr()}+"' is already registered");
    } else {
        auto res = set.emplace(recvmessage.getSender().cStr());
        // pair of iterator, bool
        if(res.second){
            response.setType(Plumbing::Type::ACK);
        } else {
            response.setType(Plumbing::Type::ERROR);
            response.setMessage("Could not add to worker list, sorry");
        }
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
    spdlog::drop_all();
}

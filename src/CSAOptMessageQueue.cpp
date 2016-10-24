#include "CSAOptMessageQueue.h"
#include <future>
#include <sstream>
#include "kj/KjStringPipe.h"

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
            ; // Just handle messages
        }
    }
}

void CSAOpt::MessageQueue::runPlumbingRepReqLoop(std::string host, unsigned int port) {
    this->logger->info("Entering Plumbing REP/REQ loop");

    memberMap members;

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

            this->logger->info("Plumbing rep/req loop message deserialized");

            Plumbing::Reader recvPlumbing = recvMessage.getRoot<Plumbing>();

            this->logger->info("Received plumbing with Id {}", recvPlumbing.getId().cStr());


            ::capnp::MallocMessageBuilder message;
            Plumbing::Builder plumbing = message.initRoot<Plumbing>();

            switch (recvPlumbing.getType()) {
                case Plumbing::Type::REGISTER:
                    handleRegister(plumbing, recvPlumbing, members);
                    break;
                case Plumbing::Type::UNREGISTER:
                    handleUnregister(plumbing, recvPlumbing, members);
                    break;
                case Plumbing::Type::HEARTBEAT:
                    handleHeartbeat(plumbing, recvPlumbing, members);
                    break;
                case Plumbing::Type::STATS:
                    handleStats(plumbing, members);
                    break;
                default:
//                    this->logger->error("Unrecognized Message type: {}", recvPlumbing.getType());
                    break;
            }

            pipe.clear();
            writePackedMessage(pipe, message);

            logger->info("Sending response for id {}", plumbing.getId().cStr());

            zmqpp::message resp;
            resp << pipe.getData();

            socket.send(resp);

            handleWorkerTimeouts(members);
        }
    }
}
void CSAOpt::MessageQueue::handleUnregister(Plumbing::Builder& response,
                                            Plumbing::Reader& recvmessage,
                                            memberMap& members) {
    response.setId(recvmessage.getId());
    response.setTimestamp(time_t(0));
    auto it = members.find(std::string{recvmessage.getSender().cStr()});
    if(it == members.end()){
        response.setType(Plumbing::Type::ERROR);
        response.setMessage("Worker '"+std::string{recvmessage.getSender().cStr()}+"' is not registered");
    } else {
        members.erase(it);
        response.setType(Plumbing::Type::ACK);
    }
}


void CSAOpt::MessageQueue::handleRegister(Plumbing::Builder& response,
                                          Plumbing::Reader& recvmessage,
                                          memberMap& members) {
    response.setId(recvmessage.getId());
    response.setTimestamp(time_t(0));
    if(members.count(recvmessage.getSender().cStr()) > 0){
        response.setType(Plumbing::Type::ERROR);
        response.setMessage("Worker '"+std::string{recvmessage.getSender().cStr()}+"' is already registered");
    } else {
        auto res = members.emplace(recvmessage.getSender().cStr(), std::chrono::system_clock::now());
        // pair of iterator, bool
        if(res.second){
            response.setType(Plumbing::Type::ACK);
        } else {
            response.setType(Plumbing::Type::ERROR);
            response.setMessage("Could not add to worker list, sorry");
        }
    }
}

void CSAOpt::MessageQueue::handleHeartbeat(Plumbing::Builder &response,
                                           Plumbing::Reader& recvmessage,
                                           memberMap& members) {
    if(members.count(recvmessage.getSender().cStr()) == 0){
        response.setType(Plumbing::Type::ERROR);
        response.setMessage("Worker '"+std::string{recvmessage.getSender().cStr()}+"' is not registered");
    } else {

    }
}

void CSAOpt::MessageQueue::handleWorkerTimeouts(CSAOpt::memberMap members) {
    auto now = std::chrono::system_clock::now();
    for(auto it = members.cbegin(); it != members.cend(); ) {
        auto heartbeat = it->second;
        auto msElapsedSinceHB = std::chrono::duration_cast<std::chrono::milliseconds>(now - heartbeat);
        if(msElapsedSinceHB > this->heartbeatTimeout) {
            members.erase(it);
        }
        it++;
    }
}

void CSAOpt::MessageQueue::handleStats(Plumbing::Builder &builder, memberMap &members) {
    ;
}

CSAOpt::MessageQueue::~MessageQueue(){
    this->logger->debug("Destructor for MessageQueue");
    this->run = false;

    std::future<void> future = std::async ([=]{
        this->logger->debug("Destructor waiting for threads to join");
        this->plumbingRepReqThread.join();
        this->tidingsRepReqThread.join();
    });

    std::chrono::seconds myTimeout(2);
    if(future.wait_for(myTimeout) == std::future_status::timeout){
        this->logger->warn("Destructor for MessageQueue timed out waiting for threads to join.");
    } else {
        this->logger->debug("Exiting MessageQueue.");
    }

    spdlog::drop_all();
}


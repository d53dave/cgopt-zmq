#include <future>
#include <sstream>
#include "CSAOptMessageQueue.h"
#include "kj/KjStringPipe.h"


namespace CSAOpt {

    std::mutex mutex;


    MessageQueue::MessageQueue(unsigned int tidingsPort, unsigned int plumbingPort) {
        std::vector<spdlog::sink_ptr> sinks;

        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        sinks.push_back(
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
        this->logger = std::make_shared<spdlog::logger>("csaopt-zmq-logger", begin(sinks), end(sinks));
        spdlog::register_logger(this->logger);
        this->logger->set_level(spdlog::level::debug);

        std::string host{"*"};
        this->run = true;

        this->statsGatherer = StatsGatherer();

        this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

        this->plumbingRepReqThread = std::thread([=] { runPlumbingRepReqLoop(host, plumbingPort); });
        this->tidingsRepReqThread = std::thread([=] { runTidingsRepReqLoop(host, tidingsPort); });
        this->statsThread = std::thread([=] { computeStatsLoop(); });

        this->logger->info("Started threads REQ/REP threads");

//        this->plumbingRepReqThread.detach();
//        this->tidingsRepReqThread.detach();
//        this->statsThread.detach();
    };

    void MessageQueue::runTidingsRepReqLoop(std::string host, unsigned int port) {
        this->logger->info("Entering Tiding REP/REQ loop");

        zmqpp::context context;
        zmqpp::socket_type type = zmqpp::socket_type::rep;
        zmqpp::socket socket{
                context,
                type
        };

        const zmqpp::endpoint_t endpoint = fmt::format("tcp://{}:{}", host, port);
        this->logger->info("Binding socket on {}", endpoint);
        socket.bind(endpoint);

        zmqpp::poller poller;
        poller.add(socket);

        while (this->run) {
            if (poller.poll(100)) { ; // Just handle messages
                logger->info("Tidings rep/req loop pass");
                zmqpp::message req;
                socket.receive(req);
                auto begin = std::chrono::high_resolution_clock::now();

                logger->info("Tidings rep/req loop recv message");
                std::string strBuf;
                req >> strBuf;

                kj::std::StringPipe pipe(strBuf);
                ::capnp::PackedMessageReader recvMessage(pipe);

                this->logger->info("Tidings rep/req loop message deserialized");

                Tidings::Reader recvTiding = recvMessage.getRoot<Tidings>();

                this->logger->info("Received tiding with Id {}", recvTiding.getId().cStr());


                ::capnp::MallocMessageBuilder message;
                Tidings::Builder tiding = message.initRoot<Tidings>();

                tiding.setId(recvTiding.getId());
                tiding.setTimestamp(time_t(0));

                // TODO: handle actual message

                pipe.clear();
                writePackedMessage(pipe, message);

                logger->info("Sending response for tiding with id {}", tiding.getId().cStr());

                zmqpp::message resp;
                resp << pipe.getData().c_str();


                socket.send(resp);

                auto end = std::chrono::high_resolution_clock::now();
                saveResponseTime(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
            }
        }
    }

    void MessageQueue::runPlumbingRepReqLoop(std::string host, unsigned int port) {
        this->logger->info("Entering Plumbing REP/REQ loop");

        memberMap members;

        zmqpp::context context;

        const zmqpp::socket_type type = zmqpp::socket_type::rep;
        zmqpp::socket socket{context, type};

        const zmqpp::endpoint_t endpoint = fmt::format("tcp://{}:{}", host, port);
        this->logger->info("Binding socket on {}", endpoint.c_str());
        socket.bind(endpoint);

        zmqpp::poller poller;
        poller.add(socket);

        while (this->run) {
            if (poller.poll(100)) {
                logger->info("Plumbing rep/req loop pass");
                zmqpp::message req;
                socket.receive(req);
                auto begin = std::chrono::high_resolution_clock::now();

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

                plumbing.setId(recvPlumbing.getId());
                plumbing.setTimestamp(time_t(0));

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

                logger->info("Sending response for plumbing with id {}", plumbing.getId().cStr());

                zmqpp::message resp;
                resp << pipe.getData().c_str();

                socket.send(resp);
                auto end = std::chrono::high_resolution_clock::now();
                saveResponseTime(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());

                handleWorkerTimeouts(members);
            }
        }
    }

    static size_t lastResponseTimeIdx = 0;

    void MessageQueue::saveResponseTime(long microsecs) {
        this->durationsMicrosecs[lastResponseTimeIdx++] = microsecs;
        if (lastResponseTimeIdx > this->responseTimeAvgCount) {
            lastResponseTimeIdx = 0;
        }
    }

    void MessageQueue::handleUnregister(Plumbing::Builder &response,
                                        Plumbing::Reader &recvmessage,
                                        memberMap &members) {

        auto it = members.find(std::string{recvmessage.getSender().cStr()});
        if (it == members.end()) {
            response.setType(Plumbing::Type::ERROR);
            response.setMessage("Worker '" + std::string{recvmessage.getSender().cStr()} + "' is not registered");
        } else {
            members.erase(it);
            response.setType(Plumbing::Type::ACK);
        }
    }


    void MessageQueue::handleRegister(Plumbing::Builder &response,
                                      Plumbing::Reader &recvmessage,
                                      memberMap &members) {
        if (members.count(recvmessage.getSender().cStr()) > 0) {
            response.setType(Plumbing::Type::ERROR);
            response.setMessage("Worker '" + std::string{recvmessage.getSender().cStr()} + "' is already registered");
        } else {
            auto res = members.emplace(recvmessage.getSender().cStr(), std::chrono::system_clock::now());
            // pair of iterator, bool
            if (res.second) {
                response.setType(Plumbing::Type::ACK);
            } else {
                response.setType(Plumbing::Type::ERROR);
                response.setMessage("Could not add to worker list, sorry");
            }
        }
    }

    void MessageQueue::handleHeartbeat(Plumbing::Builder &response,
                                       Plumbing::Reader &recvmessage,
                                       memberMap &members) {
        if (members.count(recvmessage.getSender().cStr()) == 0) {
            response.setType(Plumbing::Type::ERROR);
            response.setMessage("Worker '" + std::string{recvmessage.getSender().cStr()} + "' is not registered");
        } else {
            members[recvmessage.getSender().cStr()] = std::chrono::system_clock::now();
            response.setType(Plumbing::Type::ACK);
        }
    }

    void MessageQueue::handleWorkerTimeouts(CSAOpt::memberMap members) {
        auto now = std::chrono::system_clock::now();
        for (auto it = members.cbegin(); it != members.cend();) {
            auto heartbeat = it->second;
            auto msElapsedSinceHB = std::chrono::duration_cast<std::chrono::milliseconds>(now - heartbeat);
            if (msElapsedSinceHB > this->heartbeatTimeout) {
                logger->warn("Worker {} has timed out ({}ms > {}ms)", it->first, msElapsedSinceHB.count(),
                             this->heartbeatTimeout.count());
                members.erase(it);
            }
            it++;
        }
    }

    void MessageQueue::handleStats(Plumbing::Builder &response, memberMap const &members) {
        Stats &stats = this->getCurrentStats();

        stats.numWorkers = members.size();
        stats.queueSizeTidings = this->workQueue.size();

        double avgTime = 0;
        for (auto &&val : this->durationsMicrosecs) {
            if (val > 0) {
                double v = val;
                avgTime += v / 2;
            }
        }

        // Send results
        response.setType(Plumbing::Type::ACK);
//TODO:        response.setStats();

    }

    MessageQueue::~MessageQueue() {
        this->logger->debug("Destructor for MessageQueue");
        this->logger->flush();
        this->run = false;

        std::future<void> future = std::async([=] {
            this->logger->debug("Destructor waiting for threads to join");
            this->logger->flush();
            this->plumbingRepReqThread.join();
            this->tidingsRepReqThread.join();
            this->statsThread.join();
        });

        std::chrono::seconds myTimeout(2);
        if (future.wait_for(myTimeout) == std::future_status::timeout) {
            this->logger->warn("Destructor for MessageQueue timed out waiting for threads to join.");
            this->logger->flush();
        } else {
            this->logger->debug("Exiting MessageQueue.");
            this->logger->flush();
        }

        spdlog::drop_all();
    }


    void MessageQueue::computeStatsLoop() {
        std::chrono::milliseconds sleepPeriod(500);
        while (this->run) {
            std::this_thread::sleep_for(sleepPeriod);
            Stats stats = this->statsGatherer.computeStats();
            std::lock_guard<std::mutex> guard(mutex);
            this->currentStats = stats;
        }
    }

    Stats &MessageQueue::getCurrentStats() {
        std::lock_guard<std::mutex> guard(mutex);  // This now locked your mutex
        return this->currentStats;
    }

}

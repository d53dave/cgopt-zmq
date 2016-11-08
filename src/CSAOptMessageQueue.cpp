#include <future>
#include <sstream>
#include "CSAOptMessageQueue.h"
#include "kj/KjStringPipe.h"

#ifdef __linux__
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif


namespace CSAOpt {

    MessageQueue::MessageQueue(int tidingsPort, int plumbingPort) {
        std::vector<spdlog::sink_ptr> sinks;

        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        sinks.push_back(
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
        this->logger = std::make_shared<spdlog::logger>("csaopt-zmq-logger", begin(sinks), end(sinks));
        spdlog::register_logger(this->logger);

        std::string host{"*"};
        this->run = true;

        this->logger->info("Messagequeue started on with ports {} and {}", tidingsPort, plumbingPort);

        this->plumbingRepReqThread = std::thread([=] { runPlumbingRepReqLoop(host, plumbingPort); });
        this->tidingsRepReqThread = std::thread([=] { runTidingsRepReqLoop(host, tidingsPort); });

        this->logger->info("Started threads REQ/REP threads");
    };

    void MessageQueue::runTidingsRepReqLoop(std::string host, unsigned int port) {
        this->logger->info("Entering Tiding REP/REQ loop");

        zmqpp::context context;
        zmqpp::socket_type type = zmqpp::socket_type::rep;
        zmqpp::socket socket(context, type);

        std::string addr = "tcp://" + host + ":" + std::to_string(port);
        this->logger->info("Binding socket on {}", addr);
        socket.bind(addr.c_str());

        zmqpp::poller poller;
        poller.add(socket);

        while (this->run) {
            if (poller.poll(100)) { ; // Just handle messages

            }
        }
    }

    void MessageQueue::runPlumbingRepReqLoop(std::string host, unsigned int port) {
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

        while (this->run) {
            if (poller.poll(100)) {
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

                logger->info("Sending response for id {}", plumbing.getId().cStr());

                zmqpp::message resp;
                resp << pipe.getData();

                socket.send(resp);

                handleWorkerTimeouts(members);
            }
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
                members.erase(it);
            }
            it++;
        }
    }


    void getTotalVirtualMemory(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        sysinfo (&memInfo);
        long long totalVirtualMem = memInfo.totalram;
        totalVirtualMem += memInfo.totalswap;
        totalVirtualMem *= memInfo.mem_unit;
        stats.totalVirtualMemory = totalVirtualMem;
#endif
    }

    void getTotalPhysicalMemory(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__

        long long totalPhysMem = memInfo.totalram;
        totalPhysMem *= memInfo.mem_unit;
        stats.totalPhysicalMemory = totalPhysMem;
#endif
    }

    void getUsedVirtualMemory(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
        //Add other values in next statement to avoid int overflow on right hand side...
        virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
        virtualMemUsed *= memInfo.mem_unit;
        stats.usedVirtualMemory = virtualMemUsed;
#endif
    }

    void getUsedPhysicalMemory(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        long long physMemUsed = memInfo.totalram - memInfo.freeram;
        physMemUsed *= memInfo.mem_unit;
#endif
    }

    void getOwnUsedVirtualMemory(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmSize:", 7) == 0){
                int i = strlen(line);
                const char* p = line;
                while (*p <'0' || *p > '9') {
                    p++;
                }
                line[i-3] = '\0';
                result = atoi(p);
                break;
            }
        }
        fclose(file);
        stats.usedVirtualMemoryUsedByMe = result;
#endif
    }

    void getOwnUsedVirtualMemory2(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmSize:", 7) == 0){
                int i = strlen(line);
                const char* p = line;
                while (*p <'0' || *p > '9') {
                    p++;
                }
                line[i-3] = '\0';
                result = atoi(p);
                break;
            }
        }
        fclose(file);
        stats.usedVirtualMemoryUsedByMe = result;
#endif
    }

    void getCPULoad(struct sysinfo& memInfo, Stats& stats) {
#ifdef __linux__
        stats.usedCPU = result;
        stats.usedCPUbyMe = myCPU;
#endif
    }

    void MessageQueue::handleStats(Plumbing::Builder &response, memberMap &members) {

        Stats stats{-1, -1, -1, -1, -1, -1.0, -1.0, -1, -1};

#ifdef __linux__
        struct sysinfo memInfo;
        sysinfo (&memInfo);

        getTotalVirtualMemory(memInfo, stats);
        getTotalPhysicalMemory(memInfo, stats);
        getUsedPhysicalMemory(memInfo, stats);
        getUsedVirtualMemory(memInfo, stats)
        getOwnUsedVirtualMemory(memInfo, stats);
        getUsedVirtualMemory(memInfo, stats);
        getCPU(stats);
        resolveQueueSizes(stats);
        resolveWorkerCount(stats);
#endif

        // Send results
        response.setType(Plumbing::Type::ACK);
        response.setStats();

    }

    MessageQueue::~MessageQueue() {
        this->logger->debug("Destructor for MessageQueue");
        this->run = false;

        std::future<void> future = std::async([=] {
            this->logger->debug("Destructor waiting for threads to join");
            this->plumbingRepReqThread.join();
            this->tidingsRepReqThread.join();
        });

        std::chrono::seconds myTimeout(2);
        if (future.wait_for(myTimeout) == std::future_status::timeout) {
            this->logger->warn("Destructor for MessageQueue timed out waiting for threads to join.");
        } else {
            this->logger->debug("Exiting MessageQueue.");
        }

        spdlog::drop_all();
    }

}

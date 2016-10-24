//
// Created by David Sere on 19/07/16.
//

#pragma once

#include <queue>
#include <map>
#include <tidings/tidings.capnp.h>
#include <spdlog/spdlog.h>
#include <zmqpp/zmqpp.hpp>
#include <capnp/serialize-packed.h>
#include <set>
#include <tidings/plumbing.capnp.h>

namespace CSAOpt {

    typedef std::string workerId;
    typedef std::chrono::system_clock::time_point heartbeat;
    typedef std::map<workerId, heartbeat> memberMap;

    class MessageQueue {
    public:
        explicit MessageQueue(int tidingsPort, int plumbingsPort);
        ~MessageQueue();
    private:
        void runTidingsRepReqLoop(std::string host, unsigned int port);
        void runPlumbingRepReqLoop(std::string host, unsigned int port);

        std::thread plumbingRepReqThread;
        std::thread tidingsRepReqThread;

        std::shared_ptr<spdlog::logger> logger;

        std::chrono::milliseconds heartbeatTimeout;

        volatile bool run;

        void handleRegister(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleUnregister(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleHeartbeat(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleStats(Plumbing::Builder& builder, memberMap& members);

        void handleWorkerTimeouts(memberMap map);
    };

}



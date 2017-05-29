//
// Created by David Sere on 19/07/16.
//

#pragma once

#include <queue>
#include <map>
#include <tidings/tidings.capnp.h>
#include <spdlog/spdlog.h>
#include <zmq.h>
#include <zmqpp/zmqpp.hpp>
#include <capnp/serialize-packed.h>
#include <tidings/plumbing.capnp.h>
#include "StatsGatherer.h"

namespace CSAOpt {

    typedef std::string workerId;
    typedef std::chrono::system_clock::time_point heartbeat;
    typedef std::map<workerId, heartbeat> memberMap;


    class MessageQueue {
    public:
        explicit MessageQueue(unsigned int tidingsPort, unsigned int plumbingsPort);
        void logDebug();
        ~MessageQueue();
    private:
        static constexpr size_t responseTimeAvgCount = 3;
        long durationsMicrosecs[responseTimeAvgCount] = {0};
        void saveResponseTime(long d);

//        std::vector<std::string> workQueue = {};
//        std::map<std::string, std::string> results;

        void runTidingsRepReqLoop(std::string host, unsigned int port);
        void runPlumbingRepReqLoop(std::string host, unsigned int port);

        std::thread plumbingRepReqThread;
        std::thread tidingsRepReqThread;
        std::thread statsThread;

        std::shared_ptr<spdlog::logger> logger;

        Stats currentStats;
        Stats& getCurrentStats();

        std::chrono::milliseconds heartbeatTimeout;

        volatile bool run;


        void handleRegister(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleUnregister(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleHeartbeat(Plumbing::Builder& builder, Plumbing::Reader& reader, memberMap& members);
        void handleStats(Plumbing::Builder& builder, memberMap const& members);

        void handleWorkerTimeouts(memberMap& map);

        void computeStatsLoop();
    };
}

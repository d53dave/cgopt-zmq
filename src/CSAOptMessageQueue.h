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

namespace CSAOpt {

    class MessageQueue {
    public:
        explicit MessageQueue(int tidingsPort, int plumbingsPort);
        ~MessageQueue();
    private:
        void runTidingsRepReqLoop(std::string host, unsigned int port);
        void runPlumbingRepReqLoop(std::string host, unsigned int port);
        void readMessageToTmpFile(zmqpp::socket& socket, const std::FILE* file) const;

        std::thread plumbingRepReqThread;
        std::thread tidingsRepReqThread;

        std::shared_ptr<spdlog::logger> logger;

        volatile bool run;
    };

}



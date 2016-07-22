//
// Created by David Sere on 19/07/16.
//

#pragma once

#import <queue>
#import <map>
#import <tidings/tidings.capnp.h>
#import <spdlog/spdlog.h>
#import <zmqpp/zmqpp.hpp>

namespace CSAOpt {

    class MessageQueue {
    public:
        explicit MessageQueue(int tidingsPort, int plumbingsPort);
    private:
        void startMessageHandling(zmqpp::socket &tidingSocket, zmqpp::socket &plumbingSocket);
        void runTidingsRepReqLoop(std::string host, unsigned int port);
        void runPlumbingRepReqLoop(std::string host, unsigned int port);

        std::shared_ptr<spdlog::logger> logger;

        volatile bool run;
    };

}



//
// Created by David Sere on 19/07/16.
//

#pragma once

#import <queue>
#import <map>
#import <tidings/tiding.capnp.h>
#import <spdlog/spdlog.h>

namespace CSAOpt {

    class MessageQueue {
    public:
        MessageQueue(int port);
    private:
        std::shared_ptr<spdlog::logger> logger;
        std::queue<Tiding> work;
    };

}



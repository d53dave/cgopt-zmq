#include "CSAOptMessageQueue.h"
#include <sstream>
#include <csignal>

namespace {
    volatile std::sig_atomic_t gSignalStatus;
}

CSAOpt::MessageQueue *msgQueue;

void signal_handler(int signal) {
    gSignalStatus = signal;
//    if(msgQueue) {
//        msgQueue->abort();
//    }
}

int main(int argc, char* argv[]) {
    int port = 11551;
    int port2 = 12559;
    if(argc > 2) {
        port = std::stoi(std::string{argv[1]});
        port2 = std::stoi(std::string{argv[2]});
    }

    std::signal(SIGINT, signal_handler);

    msgQueue = new CSAOpt::MessageQueue(port, port2);

    while(gSignalStatus == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Deleting msgQueue" << std::endl;
    delete msgQueue;
    std::cout << "Done." << std::endl;
}

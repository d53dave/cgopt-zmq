#include "CSAOptMessageQueue.h"
#include <sstream>
#include <csignal>

namespace {
    volatile std::sig_atomic_t gSignalStatus;
}

CSAOpt::MessageQueue *msgQueue;

void signal_handler(int signal) {
    gSignalStatus = signal;
}

int main(int argc, char* argv[]) {
    unsigned int port = 11551;
    unsigned int port2 = 12559;
    if(argc > 2) {
        port = (unsigned int) std::stoi(std::string{argv[1]});
        port2 = (unsigned int) std::stoi(std::string{argv[2]});
    }


    std::signal(SIGINT, signal_handler);

    msgQueue = new CSAOpt::MessageQueue(port, port2);

    std::vector<std::string> arguments(argv, argv + argc);
    for(auto && arg : arguments) {
        if(arg == "-v" || arg == "--verbose") {
            msgQueue->logDebug();
        }
    }

    while(gSignalStatus == 0 && !msgQueue->finished) {
        // just sit around and wait for exit
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delete msgQueue;
    std::cout << "Bye." << std::endl;
}

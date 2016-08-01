#include "CSAOptMessageQueue.h"
#include <tidings/plumbing.capnp.h>
#include <sstream>
#include "KJ/KjStringPipe.h"

int main(int argc, char* argv[]) {
    int port = 11551;
    int port2 = 12559;
    if(argc > 2) {
        port = std::stoi(std::string{argv[1]});
        port2 = std::stoi(std::string{argv[2]});
    }

    CSAOpt::MessageQueue messageQueue(port, port2);
}

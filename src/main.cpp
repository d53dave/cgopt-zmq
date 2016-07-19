#include "CSAOptMessageQueue.h"
#include <string>

int main(int argc, char* argv[]) {
    int port = 5555;
    if(argc > 1) {
        port = std::stoi(std::string{argv[1]});
    }

    CSAOpt::MessageQueue messageQueue(port);
}

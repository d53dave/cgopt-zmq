//
// Created by David Sere on 25/07/16.
//

#include <tidings/plumbing.capnp.h>
#include "TestConfig.h"
#include "../src/CSAOptMessageQueue.h"
#include <string>
#include <ext/stdio_filebuf.h>

SCENARIO("Registering as a worker should return OK", "") {

    int tidingsPort = 12345;
    int plumbingsPort = 12378;

    GIVEN("A messagequeue") {

        CSAOpt::MessageQueue messageQueue(tidingsPort, plumbingsPort);

        ::capnp::MallocMessageBuilder message;
        Plumbing::Builder plumbing = message.initRoot<Plumbing>();

        std::time_t t = std::time(0);

        zmqpp::context context;
        zmqpp::socket_type type = zmqpp::socket_type::req;
        zmqpp::socket socket(context, type);

        const std::string endpoint = "tcp://localhost:" + std::to_string(plumbingsPort);

        socket.connect(endpoint);

        WHEN("A worker wants to join"){

            zmqpp::message req;

            plumbing.setId("1234");
            plumbing.setTimestamp(t);
            plumbing.setSender("worker1");
            plumbing.setType(Plumbing::Type::REGISTER);

            std::FILE* tmpf = std::tmpfile();
            int fd = fileno(tmpf);
            writePackedMessageToFd(fd, message);

            const size_t N=1024;
            size_t read = N;
            std::string total;
            while (read >= N) {
                std::vector<char> buf[N];
                fread((void *)&buf[0], 1, N, tmpf);
                if (read) {
                    total.append(buf->begin(), buf->end());
                }
            }

            req << total;

            socket.send(req);

            THEN("The server responds with OK"){
                req.reset_read_cursor();
                socket.receive(req);

                std::string serialized_res;

                req >> serialized_res;

                std::ofstream file("/tmp/capnp.bin");



            }
        }

        WHEN("A worker want's to leave after it jointed"){
            THEN("The server responds with OK"){

            }
        }

        WHEN("A worker want's to leave but never joined"){
            THEN("The server responds with Error"){

            }
        }

    }

}
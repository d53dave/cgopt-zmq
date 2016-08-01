//
// Created by David Sere on 25/07/16.
//

#include <tidings/plumbing.capnp.h>
#include "TestConfig.h"
#include "../src/CSAOptMessageQueue.h"
#include "../src/kj/KjStringPipe.h"

SCENARIO("Workers want to register and unregister with the Messagequeue", "") {

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
        std::cout << "Socket connected to"<< endpoint << std::endl;

        WHEN("A worker wants to join"){

            zmqpp::message req;

            plumbing.setId("1234");
            plumbing.setTimestamp(t);
            plumbing.setSender("worker1");
            plumbing.setType(Plumbing::Type::REGISTER);

            kj::std::StringPipe pipe;

            writePackedMessage(pipe, message);

            req << pipe.getData();
            std::cout << "Data from pipe: "<< pipe.getData() << std::endl;
            socket.send(req);

            THEN("The server responds with OK"){
                zmqpp::message resp;
                socket.receive(resp);

                std::string recvdata;

                resp >> recvdata;

                kj::std::StringPipe newPipe(recvdata);

                ::capnp::PackedMessageReader recvMessage(newPipe);

                Plumbing::Reader recvPlumbing = recvMessage.getRoot<Plumbing>();
//                std::cout << "Round trip id="<<recvPlumbing.getId().cStr() << " and type "<< recvPlumbing.getType() std::endl;

                REQUIRE(strcmp(plumbing.getId().cStr(), recvPlumbing.getId().cStr()) == 0);
                REQUIRE(recvPlumbing.getType() == Plumbing::Type::ACK);
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
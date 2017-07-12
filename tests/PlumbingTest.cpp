//
// Created by David Sere on 25/07/16.
//

#include <tidings/plumbing.capnp.h>
#include "TestConfig.h"
#include "../src/CSAOptMessageQueue.h"
#include "../src/kj/KjStringPipe.h"

void setData(Plumbing::Builder p, std::string id, std::time_t timestamp, std::string sender, Plumbing::Type type){
    p.setId(id); p.setTimestamp(timestamp); p.setSender(sender); p.setType(type);
}

SCENARIO("Workers want to register and unregister with the Messagequeue", "") {
    int tidingsPort = 12345;
    int plumbingsPort = 12378;

    GIVEN("A messagequeue") {
        CSAOpt::MessageQueue* messageQueue = new CSAOpt::MessageQueue(tidingsPort, plumbingsPort);

        zmqpp::context context;
        zmqpp::socket_type type = zmqpp::socket_type::req;
        zmqpp::socket socket(context, type);

        const std::string endpoint = "tcp://localhost:" + std::to_string(plumbingsPort);

        socket.connect(endpoint);

        WHEN("A worker wants to join"){
            zmqpp::message req;

            ::capnp::MallocMessageBuilder message;
            Plumbing::Builder plumbing = message.initRoot<Plumbing>();

            std::time_t t = std::time(0);
            setData(plumbing, "1234", t, "worker1", Plumbing::Type::REGISTER);

            kj::std::StringPipe pipe;
            writePackedMessage(pipe, message);

            req << pipe.getData();

            socket.send(req);

            THEN("The server responds with OK"){
                zmqpp::message resp;
                socket.receive(resp);

                std::string recvdata;
                resp >> recvdata;

                kj::std::StringPipe newPipe(recvdata);
                ::capnp::PackedMessageReader recvMessage(newPipe);
                Plumbing::Reader recvPlumbing = recvMessage.getRoot<Plumbing>();


                REQUIRE(strcmp(plumbing.getId().cStr(), recvPlumbing.getId().cStr()) == 0);
                REQUIRE(recvPlumbing.getType() == Plumbing::Type::ACK);
            }
        }

        WHEN("A worker wants to leave after it jointed"){
            zmqpp::message req;

            ::capnp::MallocMessageBuilder message;
            Plumbing::Builder plumbing = message.initRoot<Plumbing>();

            std::time_t t = std::time(0);
            setData(plumbing, "1234", t, "worker1", Plumbing::Type::REGISTER);

            kj::std::StringPipe pipe;
            writePackedMessage(pipe, message);

            // send 'register'
            req << pipe.getData();
            socket.send(req);

            // ignore response
            zmqpp::message resp;
            socket.receive(resp);

            // send 'unregister'
            ::capnp::MallocMessageBuilder message2;
            Plumbing::Builder plumbing2 = message2.initRoot<Plumbing>();

            std::time_t t2 = std::time(0);
            setData(plumbing2, "1234", t2, "worker1", Plumbing::Type::UNREGISTER);
            pipe.clear();
            writePackedMessage(pipe, message2);

            req << pipe.getData();
            socket.send(req);

            THEN("The server responds with OK"){
                zmqpp::message resp;
                socket.receive(resp);

                std::string recvdata;
                resp >> recvdata;

                kj::std::StringPipe newPipe(recvdata);
                ::capnp::PackedMessageReader recvMessage(newPipe);

                Plumbing::Reader recvPlumbing = recvMessage.getRoot<Plumbing>();

                REQUIRE(strcmp(plumbing.getId().cStr(), recvPlumbing.getId().cStr()) == 0);
                REQUIRE(recvPlumbing.getType() == Plumbing::Type::ACK);
            }
        }

        WHEN("A worker want's to leave but never joined"){
            zmqpp::message reqBad;

            ::capnp::MallocMessageBuilder messageBad;
            Plumbing::Builder plumbingBad = messageBad.initRoot<Plumbing>();

            std::time_t tBad = std::time(0);
            setData(plumbingBad, "1239", tBad, "worker7", Plumbing::Type::UNREGISTER);

            kj::std::StringPipe pipeBad;
            writePackedMessage(pipeBad, messageBad);

            reqBad << pipeBad.getData();

            socket.send(reqBad);

            THEN("The server responds with Error"){
                zmqpp::message respBad;
                socket.receive(respBad);

                std::string recvdataBad;
                respBad >> recvdataBad;

                kj::std::StringPipe newPipeBad(recvdataBad);
                ::capnp::PackedMessageReader recvMessageBad(newPipeBad);
                Plumbing::Reader recvPlumbingBad = recvMessageBad.getRoot<Plumbing>();


                REQUIRE(strcmp(plumbingBad.getId().cStr(), recvPlumbingBad.getId().cStr()) == 0);
                REQUIRE(recvPlumbingBad.getType() == Plumbing::Type::ERROR);
            }
        }

        delete messageQueue;
    }
}
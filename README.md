# csaopt-kafka [![Build Status](https://travis-ci.org/d53dave/csaopt-zmq.svg?branch=master)](https://travis-ci.org/d53dave/csaopt-zmq) [![Coverage Status](https://coveralls.io/repos/github/d53dave/csaopt-zmq/badge.svg?branch=master)](https://coveralls.io/github/d53dave/csaopt-zmq?branch=master)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fd53dave%2Fcsaopt-kafka.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fd53dave%2Fcsaopt-kafka?ref=badge_shield)

# THIS PROJECT HAS BEEN SUNSET IN FAVOR OF [WURSTMEISTER/KAFKA](https://hub.docker.com/r/wurstmeister/kafka). REST IN PIECE.


This is the messagequeue that CSAOpt uses to communicate between the client and worker nodes. More
speficically, this repo hosts the Dockerfile for Kafka and Zookeeper (inspired by 
[spotify/kafka](https://github.com/spotify/docker-kafka)).

## Changelog

>0.2.0 Pivot to Kafka

There really is no use in implementing this myself (except for learning and research). I have learnt something
quite important: **don't reinvent the wheel**.
Make no mistake, dumping C++ was a very good choice, and implementing the functionality (though not complete)
in Python3 and AsyncIO was a joy. But there is no point in maintaining a piece of software that does the 
same thing as others, but much worse and with a lot of not discovered bugs.

>0.1.0 Change to Python3

Starting with version 0.1.0, the C++ prototype was effectively abandoned for a re-implementation in Python3. 

>0.0.x C++ prototypes

Versions 0.0.x were prototypes written in C++, 
including the proof of concept which was demo-ed to 
my thesis supervisor and colleagues. The last version of the C++ prototype is commit [f3edf93](https://github.com/d53dave/csaopt-zmq/tree/f3edf934e383bb66a2c72f14e503dd75a04702fb).


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fd53dave%2Fcsaopt-kafka.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fd53dave%2Fcsaopt-kafka?ref=badge_large)

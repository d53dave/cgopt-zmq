# csaopt-zmq [![Build Status](https://travis-ci.org/d53dave/csaopt-zmq.svg?branch=master)](https://travis-ci.org/d53dave/csaopt-zmq) [![Coverage Status](https://coveralls.io/repos/github/d53dave/csaopt-zmq/badge.svg?branch=master)](https://coveralls.io/github/d53dave/csaopt-zmq?branch=master)

This is the messagequeue that CSAOpt uses to communicate between the client and worker nodes. As the name suggests, it is build upon (py)zmq.

Communication is done via capnproto schema.

## Changelog

>0.1.0 Change to Python3

Starting with version 0.1.0, the C++ prototype was effectively abandoned for a re-implementation in Python3. 

>0.0.x C++ prototypes

Versions 0.0.x were prototypes written in C++, 
including the proof of concept which was demo-ed to 
my thesis supervisor and colleagues. The last version of the C++ prototype is commit [f3edf93](https://github.com/d53dave/csaopt-zmq/tree/f3edf934e383bb66a2c72f14e503dd75a04702fb).
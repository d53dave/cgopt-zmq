# This will look for the zeroMQ headers and libraries and make
# appropriate variables available to CMake.

find_path(
        ZMQ_INCLUDE_DIRS
        NAMES zmq.h
        HINTS ${ZEROMQ_INCLUDE_DIRS}
)

find_library(
        ZMQ_LIBRARIES
        NAMES zmq
        HINTS ${ZEROMQ_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        ZMQ
        DEFAULT_MSG
        ZMQ_LIBRARIES
        ZMQ_INCLUDE_DIRS
)

if(ZMQ_FOUND)
    mark_as_advanced(ZMQ_LIBRARIES ZMQ_INCLUDE_DIRS)
endif()
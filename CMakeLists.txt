cmake_minimum_required(VERSION 3.5)
project(csaopt_zmq)

set(CMAKE_CXX_STANDARD 11) # adds -std=c++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories("${PROJECT_INCLUDE_DIR}")

# ZMQ
find_package(ZMQ REQUIRED)
if(ZMQ_FOUND)
    include_directories(${ZMQ_INCLUDE_DIRS})
    list(APPEND ${PROJECT_NAME}_EXTERNAL_LIBRARIES "${ZMQ_LIBRARIES}")
endif()

# ZMQPP
find_package(ZMQPP REQUIRED)
if(ZMQPP_FOUND)
    include_directories(${ZMQPP_INCLUDE_DIRS})
    list(APPEND ${PROJECT_NAME}_EXTERNAL_LIBRARIES "${ZMQPP_LIBRARIES}")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tidings)
set(CAPNPC_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/tidings)

include(ExternalProject)

ExternalProject_Add(
        csaopt-tidings
        GIT_REPOSITORY https://github.com/d53dave/csaopt-tidings.git
        TIMEOUT 10
        LOG_DOWNLOAD ON
        CMAKE_CACHE_ARGS -DCAPNPC_OUTPUT_DIR:STRING=${CAPNPC_OUTPUT_DIR}
        BUILD_COMMAND "make"
        INSTALL_COMMAND ""
)

set(SOURCE_FILES
    src/CSAOptMessageQueue.cpp src/CSAOptMessageQueue.h src/main.cpp)

add_executable(csaopt_zmq ${SOURCE_FILES} ${CAPNP_SRCS})
target_link_libraries(csaopt_zmq zmq zmqpp)
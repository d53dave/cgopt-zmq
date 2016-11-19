cmake_minimum_required(VERSION 3.2)
project(csaopt_zmq)

#set(CMAKE_CXX_COMPILER /usr/local/bin/gcc-4.9)
set(CMAKE_CXX_STANDARD 11) # adds -std=c++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories("${PROJECT_INCLUDE_DIR}")

# ZMQ
find_package(ZMQ REQUIRED)
if(ZMQ_FOUND)
    include_directories(${ZMQ_INCLUDE_DIR})
endif()

# ZMQPP
find_package(ZMQPP REQUIRED)
if(ZMQPP_FOUND)
    MESSAGE(WARNING "ZMQPPINCL=${ZMQPP_INCLUDE_DIR}")
    include_directories(${ZMQPP_INCLUDE_DIR})
endif()

find_package(CapnProto REQUIRED)

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
    src/CSAOptMessageQueue.cpp src/CSAOptMessageQueue.h src/kj/KjStringPipe.h src/StatsGatherer.h)

file(GLOB CAPNP_FILES "include/tidings/*.capnp.c++")

find_package(Threads REQUIRED)

MESSAGE(WARNING "ZMQLIB=${ZMQ_LIBRARY} and ZMQPPLIB=${ZMQPP_LIBRARY}")

add_executable(csaopt_zmq src/main.cpp ${SOURCE_FILES} ${CAPNP_SRCS})
target_link_libraries(csaopt_zmq ${ZMQ_LIBRARY} ${ZMQPP_LIBRARY} ${CAPNP_LIBRARIES} Threads::Threads)

enable_testing()
file(GLOB TEST_FILES "test/*.cpp")

#Run through each source
foreach(testFile ${TEST_FILES})
    #Extract the filename without an extension (NAME_WE)
    get_filename_component(testName ${testFile} NAME_WE)

    #Add compile target
    add_executable(${testName} ${testFile} ${SOURCE_FILES} ${CAPNP_SRCS})
    target_link_libraries(${testName} zmq zmqpp ${CAPNP_LIBRARIES})
    add_test(NAME ${testName} COMMAND ${testName} )
endforeach(testFile)
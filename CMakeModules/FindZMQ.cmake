# This will look for the ZMQPP headers and libraries and make
# appropriate variables available to CMake.

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(zmq_PKGCONF libzmq)

# Include dir
find_path(ZMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${zmq_PKGCONF_INCLUDE_DIRS})

# Finally the library itself
find_library(ZMQ_LIBRARY
        NAMES zmq
        PATHS ${zmq_PKGCONF_INCLUDE_DIRS})

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(zmq_PROCESS_INCLUDES zmq_INCLUDE_DIR )
set(zmq_PROCESS_LIBS zmq_LIBRARY )
libfind_process(zmqpp)
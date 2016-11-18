# This will look for the ZMQPP headers and libraries and make
# appropriate variables available to CMake.

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(zmqpp_PKGCONF libzmqpp)

# Include dir
find_path(ZMQPP_INCLUDE_DIR
        NAMES zmqpp/zmqpp.hpp
        PATHS ${zmqpp_PKGCONF_INCLUDE_DIRS})

# Finally the library itself
find_library(ZMQPP_LIBRARY
        NAMES zmqpp
        PATHS )

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(zmqpp_PROCESS_INCLUDES zmqpp_INCLUDE_DIR )
set(zmqpp_PROCESS_LIBS zmqpp_LIBRARY )
libfind_process(zmqpp)
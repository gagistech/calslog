cmake_minimum_required(VERSION 3.20)

# This CMakeLists.txt is for building the app using VCPKG for resolving dependencies.

# We take VCPKG_ROOT from the environment, but we have to store it in CMake cache, so that if IDE later
# needs to rerun configuration phase, it will still work even if the environment variable is not set anymore.
if(NOT VCPKG_ROOT)
    if(NOT DEFINED ENV{VCPKG_ROOT})
        message(FATAL_ERROR "VCPKG_ROOT is not set")
    endif()
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(dummy)

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

myci_add_subdirectory(../cmake
    BINARY_DIR
        app
)

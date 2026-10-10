# Sets the minimum version of CMake required to build this native library.
# This ensures that a certain set of CMake features is available to your build.
cmake_minimum_required(VERSION 3.22.1)

set(name ruisapp_application) # this has to be 'ruisapp_application' so that RuisappActivity loads that named .so
project(${name})

# The app sources live in the calslog repo (../../../ = repo root).
file(GLOB_RECURSE srcs
        "../../../src/calslog/*.cpp"
        "../../../src/calslog/*.cc"
        "../../../src/calslog/*.cxx"
        "../../../src/calslog/*.c"
)

include(${ANDROID_GRADLE_NATIVE_BUNDLE_PLUGIN_MK})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_library(
        # Specifies the name of the library.
        ${name}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${srcs}
    )

target_link_libraries(
        ${name}
        android log GLESv2 EGL z ${ANDROID_GRADLE_NATIVE_MODULES}
    )

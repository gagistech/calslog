cmake_minimum_required(VERSION 3.30)

project(calslog)

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

set(srcs)
myci_add_source_files(srcs
    DIRECTORY
        ../../src/${PROJECT_NAME}
    RECURSIVE
)

myci_declare_application(${PROJECT_NAME}-opengles
    SOURCES
        ${srcs}
    RESOURCE_DIRECTORY
        ../../res
    INCLUDE_DIRECTORIES
        ../../src
    DEPENDENCIES
        ruisapp::ruisapp-opengles
    RUN_ARGUMENTS
        --res-path=res
        --window
)

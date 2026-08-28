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

list(FILTER srcs EXCLUDE REGEX ".*/main.cpp$")

function(calslog_declare_library_and_application variant)
    myci_declare_library(lib${PROJECT_NAME}-${variant}
        SOURCES
            ${srcs}
        PRIVATE_INCLUDE_DIRECTORIES
            ../../src
        DEPENDENCIES
            clargs
            ruisapp::ruisapp-${variant}
    )

    myci_declare_application(${PROJECT_NAME}-${variant}
        SOURCES
            ../../src/${PROJECT_NAME}/main.cpp
        RESOURCE_DIRECTORY
            ../../res
        INCLUDE_DIRECTORIES
            ../../src
        DEPENDENCIES
            clargs
            ${PROJECT_NAME}::lib${PROJECT_NAME}-${variant}
        RUN_ARGUMENTS
            --res-path=res
            --window
    )
endfunction()

if(IOS)
    set(unit_test_variant opengles)
    calslog_declare_library_and_application(opengles)
elseif(APPLE)
    set(unit_test_variant opengl)
    calslog_declare_library_and_application(opengl)
else()
    set(unit_test_variant opengl)
    calslog_declare_library_and_application(opengl)
    calslog_declare_library_and_application(opengles)
endif()

# =============
# === tests ===

set(test_unit_srcs)
myci_add_source_files(test_unit_srcs
    DIRECTORY
        ../../tests/unit/src
    RECURSIVE
)

myci_declare_application(${PROJECT_NAME}-test-unit
    SOURCES
        ${test_unit_srcs}
    INCLUDE_DIRECTORIES
        ../../src
    DEPENDENCIES
        ${PROJECT_NAME}::lib${PROJECT_NAME}-${unit_test_variant}
        tst
)

myci_declare_test(
    RUN_TARGET
        run-${PROJECT_NAME}-test-unit
)

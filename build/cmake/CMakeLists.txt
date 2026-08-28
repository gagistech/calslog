cmake_minimum_required(VERSION 3.30)

project(calslog)

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

# =======================================
# === application library for testing ===

set(srcs)
myci_add_source_files(srcs
    DIRECTORY
        ../../src/${PROJECT_NAME}
    RECURSIVE
)

list(FILTER srcs EXCLUDE REGEX ".*/main.cpp$")

myci_declare_library(${PROJECT_NAME}
    SOURCES
        ${srcs}
    PRIVATE_INCLUDE_DIRECTORIES
        ../../src
    DEPENDENCIES
        clargs
        ruisapp::ruisapp-opengles
)

# =======================
# === the application ===

function(calslog_declare_application variant)
    myci_declare_application(${PROJECT_NAME}-${variant}
        SOURCES
            ../../src/${PROJECT_NAME}/main.cpp
        RESOURCE_DIRECTORY
            ../../res
        INCLUDE_DIRECTORIES
            ../../src
        DEPENDENCIES
            clargs
            ruisapp::ruisapp-${variant}
            ${PROJECT_NAME}::${PROJECT_NAME}
        RUN_ARGUMENTS
            --res-path=res
            --window
    )
endfunction()

if(APPLE AND NOT IOS)
    calslog_declare_application(opengl)
else()
    calslog_declare_application(opengles)
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
        tst
        ${PROJECT_NAME}::${PROJECT_NAME}
)

myci_declare_test(
    RUN_TARGET
        run-${PROJECT_NAME}-test-unit
)

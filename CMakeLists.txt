cmake_minimum_required(VERSION 3.15)
project(entt_ext)

set(CMAKE_CXX_STANDARD 20)

option(ENTT_EXT_BUILD_TESTS "Whether to build tests with entt_ext" OFF)
option(ENTT_EXT_PROVIDE_ENTT "Whether to provide the EnTT project and its targets" ON)

if(ENTT_EXT_PROVIDE_ENTT)
    include(FetchContent)

    FetchContent_Declare(
            EnTT
            GIT_REPOSITORY https://github.com/skypjack/entt
            GIT_TAG master
    )
    FetchContent_MakeAvailable(EnTT)
endif()

add_library(entt_ext INTERFACE)
add_library(entt_ext::entt_ext ALIAS entt_ext)

target_sources(entt_ext INTERFACE
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src/config/types.hpp>
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src/relation/relationship.hpp>
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src/system/group.hpp>
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src/system/system.hpp>
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src/entt_ext.hpp>
)

target_include_directories(
        entt_ext
        INTERFACE
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(entt_ext INTERFACE EnTT::EnTT)

if(ENTT_EXT_BUILD_TESTS)
    add_subdirectory(test)
endif()

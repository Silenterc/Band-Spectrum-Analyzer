cmake_minimum_required(VERSION 3.24)
project(band-spectrum-analyzer VERSION 0.0.1)

set(BSA_COPY_PLUGIN_AFTER_BUILD_DEFAULT ON)

if(DEFINED ENV{CI} AND NOT "$ENV{CI}" STREQUAL "")
    set(BSA_COPY_PLUGIN_AFTER_BUILD_DEFAULT OFF)
endif()

option(BSA_COPY_PLUGIN_AFTER_BUILD
    "Copy built plugins into the current user's default plugin directories after build"
    ${BSA_COPY_PLUGIN_AFTER_BUILD_DEFAULT})

add_subdirectory(modules/JUCE)
add_subdirectory(src)
add_subdirectory(tests)

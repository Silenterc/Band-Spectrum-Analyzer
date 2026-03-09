cmake_minimum_required(VERSION 3.24)
project(band-spectrum-analyzer VERSION 0.0.1)
include(CTest)
add_subdirectory(modules/JUCE)
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()

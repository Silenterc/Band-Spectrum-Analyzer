cmake_minimum_required(VERSION 3.24)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0)

FetchContent_MakeAvailable(Catch2)

add_executable(band-spectrum-analyzer-tests
    dsp/AnalyzerEngineTests.cpp
    ../src/dsp/AnalyzerEngine.cpp)

target_include_directories(band-spectrum-analyzer-tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(band-spectrum-analyzer-tests
    PRIVATE
        Catch2::Catch2WithMain
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

cmake_minimum_required(VERSION 3.24)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0)

FetchContent_MakeAvailable(Catch2)

add_executable(band-spectrum-analyzer-tests
    dsp/AnalyzerEngineTests.cpp
    ../src/dsp/core/AnalyzerEngine.cpp
    ../src/dsp/filterbank/FilterBank.cpp
    ../src/dsp/filterbank/SIMDBPFilter.cpp
    ../src/dsp/planning/AnalysisPlanBuilder.cpp
    ../src/dsp/processing/AnalysisGroupProcessor.cpp
    ../src/dsp/sources/AnalysisSourceBuilder.cpp
    ../src/ui/analyzer/helpers/AnalyzerMeter.cpp)

target_include_directories(band-spectrum-analyzer-tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(band-spectrum-analyzer-tests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(band-spectrum-analyzer-tests
    PRIVATE
        Catch2::Catch2WithMain
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

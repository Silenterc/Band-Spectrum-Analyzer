cmake_minimum_required(VERSION 3.24)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0)

FetchContent_MakeAvailable(Catch2)

add_executable(band-spectrum-analyzer-tests
    dsp/AnalyzerEngineTests.cpp
    plugin/PluginUiBridgeSettingsTests.cpp
    plugin/PresetSessionTests.cpp
    plugin/SignalOutputMixerTests.cpp
    ui/UiArchitectureRulesTests.cpp
    ui/AnalyzerUiModelTests.cpp
    ui/RasterKnobTests.cpp
    ui/RasterHorizontalSliderTests.cpp
    ui/SettingsRangeModelTests.cpp
    ../src/dsp/core/AnalyzerEngine.cpp
    ../src/dsp/filterbank/FilterBank.cpp
    ../src/dsp/filterbank/SIMDBPFilter.cpp
    ../src/dsp/planning/AnalysisPlanBuilder.cpp
    ../src/dsp/processing/AnalysisGroupProcessor.cpp
    ../src/dsp/sources/AnalysisSourceBuilder.cpp
    ../src/display/analyzer/logic/AnalyzerDisplayFrameModel.cpp
    ../src/display/analyzer/logic/AnalyzerGlobalHoldModel.cpp
    ../src/display/analyzer/logic/AnalyzerMeter.cpp
    ../src/plugin/PluginUiBridge.cpp
    ../src/plugin/SignalOutputMixer.cpp
    ../src/plugin/parameters/ParameterAccess.cpp
    ../src/plugin/presets/FactoryPresetRepository.cpp
    ../src/plugin/presets/PluginStateSerializer.cpp
    ../src/plugin/presets/PresetDocumentSerializer.cpp
    ../src/plugin/presets/PresetSession.cpp
    ../src/plugin/presets/PresetTypes.cpp
    ../src/plugin/presets/UserPresetStore.cpp
    ../src/ui/theme/UiRasterAssets.cpp
    ../src/ui/theme/UiTheme.cpp
    ../src/ui/widgets/RasterFilmstrip.cpp
    ../src/ui/widgets/RasterKnobComponent.cpp
    ../src/ui/widgets/RasterSliderValueMapping.cpp
    ../src/ui/analyzer/plot/logic/AnalyzerGeometry.cpp
    ../src/ui/analyzer/plot/logic/AnalyzerHoverModel.cpp
    ../src/ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.cpp
    ../src/ui/analyzer/plot/logic/FrequencyFormatter.cpp
    ../src/ui/analyzer/plot/logic/MusicTheory.cpp
    ../src/ui/analyzer/plot/logic/AnalyzerViewModel.cpp
    ../src/ui/analyzer/rack/model/SignalSlotOrderModel.cpp
    ../src/plugin/state/SignalSlotOrderState.cpp)

target_include_directories(band-spectrum-analyzer-tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(band-spectrum-analyzer-tests
    PRIVATE
        BSA_REPO_ROOT="${CMAKE_SOURCE_DIR}"
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(band-spectrum-analyzer-tests
    PRIVATE
        BsaUiAssets
        Catch2::Catch2WithMain
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

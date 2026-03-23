juce_add_binary_data(BsaUiAssets
    SOURCES
        ui/assets/icons/cancel.svg
        ui/assets/icons/power.svg
        ui/assets/icons/settings.svg
        ui/assets/icons/snowflake.svg
        ui/assets/uikit/background/background.png
        ui/assets/uikit/background/background_2.png
        ui/assets/uikit/background/background_2_version.png
        ui/assets/uikit/decor_grid.png
        ui/assets/uikit/pad/pad_off.png
        ui/assets/uikit/pad/pad_freeze_on.png
        ui/assets/uikit/pad/pad_on.png
        ui/assets/uikit/screen.png
        ui/assets/uikit/screw.png
        ui/assets/uikit/switch/switch_down.png
        ui/assets/uikit/switch/switch_up.png)

juce_add_plugin(${PROJECT_NAME}
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    IS_SYNTH FALSE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD ${BSA_COPY_PLUGIN_AFTER_BUILD}
    VST3_COPY_DIR "/Library/Audio/Plug-Ins/VST3"
    PLUGIN_MANUFACTURER_CODE Slnt               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Bsa0                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME ${PROJECT_NAME})        # The name of the final executable, which can differ from the target name


target_sources(${PROJECT_NAME}
    PRIVATE
        dsp/core/AnalyzerEngine.cpp
        dsp/filterbank/FilterBank.cpp
        dsp/filterbank/SIMDBPFilter.cpp
        dsp/planning/AnalysisPlanBuilder.cpp
        dsp/processing/AnalysisGroupProcessor.cpp
        dsp/sources/AnalysisSourceBuilder.cpp
        ui/UiIcons.cpp
        ui/UiRasterAssets.cpp
        ui/PadButton.cpp
        ui/SectionDividerComponent.cpp
        ui/UiTheme.cpp
        ui/EditorBackgroundComponent.cpp
        ui/analyzer/view/MainLayoutComponent.cpp
        ui/analyzer/view/AnalyzerSectionComponent.cpp
        ui/analyzer/view/AnalyzerComponent.cpp
        ui/analyzer/view/AnalyzerHoverOverlayComponent.cpp
        ui/analyzer/view/AnalyzerMeterControlsComponent.cpp
        ui/analyzer/view/SignalRackComponent.cpp
        ui/analyzer/view/SignalSlotActionButton.cpp
        ui/analyzer/view/SignalSlotComponent.cpp
        ui/analyzer/view/SignalSlotDragHandle.cpp
        ui/analyzer/view/SignalSlotModeButton.cpp
        ui/analyzer/view/SignalSlotSourceToggle.cpp
        ui/analyzer/view/SignalSlotSwatchButton.cpp
        ui/analyzer/popups/SignalColourPopupContent.cpp
        ui/analyzer/popups/SignalSelectionPopupContent.cpp
        ui/analyzer/model/AnalyzerRefreshModel.cpp
        ui/analyzer/model/AnalyzerViewModel.cpp
        ui/analyzer/model/SignalSlotOrderModel.cpp
        ui/analyzer/helpers/AnalyzerMeter.cpp
        ui/analyzer/helpers/AnalyzerGeometry.cpp
        ui/analyzer/helpers/AnalyzerHoverModel.cpp
        ui/analyzer/helpers/SignalRackDragSession.cpp
        ui/analyzer/helpers/SignalRackLayoutEngine.cpp
        ui/analyzer/helpers/FrequencyFormatter.cpp
        ui/analyzer/helpers/MusicTheory.cpp
        ui/PluginEditor.cpp
        plugin/parameters/ParameterAccess.cpp
        plugin/PluginProcessor.cpp
        plugin/state/SignalSlotOrderState.cpp)


target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
        BsaUiAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

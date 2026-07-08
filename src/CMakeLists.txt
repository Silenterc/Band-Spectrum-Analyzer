juce_add_binary_data(BsaUiAssets
    SOURCES
        ui/assets/icons/cancel.svg
        ui/assets/icons/headphones.svg
        ui/assets/icons/left.svg
        ui/assets/icons/power.svg
        ui/assets/icons/reset.svg
        ui/assets/icons/right.svg
        ui/assets/icons/save.svg
        ui/assets/icons/settings.svg
        ui/assets/icons/snowflake.svg
        ui/assets/uikit/background/background.png
        ui/assets/uikit/background/background_2.png
        ui/assets/uikit/background/background_2_version.png
        ui/assets/uikit/button/button_off.png
        ui/assets/uikit/button/button_on.png
        ui/assets/uikit/decor_grid.png
        ui/assets/uikit/hor_slider/Hor_slider.png
        ui/assets/uikit/knob_small/Knob_small.png
        ui/assets/uikit/knob_small/scale_small_knob.png
        ui/assets/uikit/logo.svg
        plugin/assets/presets/factory_default_preset.xml
        ui/assets/uikit/pad/pad_off.png
        ui/assets/uikit/pad/pad_freeze_on.png
        ui/assets/uikit/pad/pad_on.png
        ui/assets/uikit/pad_rectangle/pad_rectangle_off.png
        ui/assets/uikit/pad_rectangle/pad_rectangle_on.png
        ui/assets/uikit/screen.png
        ui/assets/uikit/screw.png
        ui/assets/uikit/switch/switch_down.png
        ui/assets/uikit/switch/switch_up.png
        ui/assets/uikit/text_box.png)

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
        display/analyzer/logic/AnalyzerDisplayFrameModel.cpp
        display/analyzer/logic/AnalyzerGlobalHoldModel.cpp
        display/analyzer/logic/AnalyzerMeter.cpp
        display/analyzer/thread/AnalyzerDisplayWorker.cpp
        ui/theme/UiIcons.cpp
        ui/theme/UiRasterAssets.cpp
        ui/widgets/RasterIconButton.cpp
        ui/widgets/RasterFilmstrip.cpp
        ui/widgets/RasterSliderValueMapping.cpp
        ui/widgets/RasterKnobComponent.cpp
        ui/widgets/RasterHorizontalSliderComponent.cpp
        ui/widgets/RasterRectanglePadButton.cpp
        ui/widgets/PadButton.cpp
        ui/widgets/CalloutPresenter.cpp
        ui/widgets/PopupActionButton.cpp
        ui/widgets/SectionDividerComponent.cpp
        ui/theme/UiTheme.cpp
        ui/editor/view/EditorBackgroundComponent.cpp
        ui/editor/layout/MainLayoutComponent.cpp
        ui/settings/layout/SettingsPageLayout.cpp
        ui/settings/view/SettingsAnalysisSectionComponent.cpp
        ui/settings/view/SettingsTimeDecaySectionComponent.cpp
        ui/settings/view/SettingsSectionFrameComponent.cpp
        ui/settings/view/SettingsPageComponent.cpp
        ui/presets/view/PresetHeaderComponent.cpp
        ui/presets/view/PresetHeaderControlsComponent.cpp
        ui/presets/view/PresetBrowserPopupComponent.cpp
        ui/presets/model/PresetActionMessageFormatter.cpp
        ui/presets/popups/PresetBrowserPopupContent.cpp
        ui/presets/view/PresetSelectorButton.cpp
        ui/presets/popups/PresetDeleteConfirmPopupContent.cpp
        ui/presets/view/PresetListRowComponent.cpp
        ui/presets/popups/PresetPopupStatusComponent.cpp
        ui/presets/popups/PresetSavePopupContent.cpp
        ui/analyzer/plot/view/AnalyzerSectionComponent.cpp
        ui/analyzer/plot/view/AnalyzerPlotComponent.cpp
        ui/analyzer/plot/view/AnalyzerHoverTooltipRenderer.cpp
        ui/analyzer/controls/view/AnalyzerMeterControlsComponent.cpp
        ui/analyzer/rack/view/SignalRackComponent.cpp
        ui/analyzer/rack/view/SignalSlotActionButton.cpp
        ui/analyzer/rack/view/SignalSlotComponent.cpp
        ui/analyzer/rack/view/SignalSlotModeButton.cpp
        ui/analyzer/rack/view/SignalSlotSourceToggle.cpp
        ui/analyzer/rack/view/SignalSlotSwatchButton.cpp
        ui/analyzer/rack/popups/SignalColourPopupContent.cpp
        ui/analyzer/rack/popups/SignalModeSelectionPopupContent.cpp
        ui/analyzer/plot/logic/AnalyzerViewModel.cpp
        ui/analyzer/rack/model/SignalSlotOrderModel.cpp
        ui/analyzer/plot/logic/AnalyzerGeometry.cpp
        ui/analyzer/plot/logic/AnalyzerHoverModel.cpp
        ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.cpp
        ui/analyzer/rack/interaction/SignalRackDragSession.cpp
        ui/analyzer/rack/interaction/SignalRackLayoutEngine.cpp
        ui/analyzer/plot/logic/FrequencyFormatter.cpp
        ui/analyzer/plot/logic/MusicTheory.cpp
        ui/editor/view/PluginEditor.cpp
        plugin/parameters/ParameterAccess.cpp
        plugin/presets/FactoryPresetRepository.cpp
        plugin/presets/PluginStateSerializer.cpp
        plugin/presets/PresetDocumentSerializer.cpp
        plugin/presets/PresetSession.cpp
        plugin/presets/PresetTypes.cpp
        plugin/presets/UserPresetStore.cpp
        plugin/ProcessorChangeTracker.cpp
        plugin/PluginUiBridge.cpp
        plugin/SignalOutputMixer.cpp
        plugin/PluginProcessor.cpp
        plugin/state/SignalSlotOrderState.cpp)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

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

cmake_minimum_required(VERSION 3.22)

project(LFOTool VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment target" FORCE)

include(FetchContent)

# JUCE is fetched at configure time. Pinning a release keeps Windows builds reproducible.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.8
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LFOTool
    COMPANY_NAME              "Arena Audio"
    BUNDLE_ID                 "com.arenaaudio.lfotool"
    PLUGIN_MANUFACTURER_CODE  Arna
    PLUGIN_CODE               Lf01
    FORMATS                   VST3
    PRODUCT_NAME              "LFO Tool"
    DESCRIPTION               "Tempo-syncable LFO / CV modulation tool for DAWs"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           "Fx|Generator"
)

juce_generate_juce_header(LFOTool)

target_sources(LFOTool
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(LFOTool
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(LFOTool
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

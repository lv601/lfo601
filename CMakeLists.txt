cmake_minimum_required(VERSION 3.22)

project(LFOzTool VERSION 2.0.0 LANGUAGES C CXX)

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

juce_add_plugin(LFOzTool
    COMPANY_NAME              "lv601"
    BUNDLE_ID                 "com.lv601.lfoztool"
    PLUGIN_MANUFACTURER_CODE  Lv60
    PLUGIN_CODE               Lz20
    FORMATS                   VST3
    PRODUCT_NAME              "LFOz Tool"
    DESCRIPTION               "LFOz Tool 2.0 by lv601 with curve editor and FL Studio CV routing"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           "Fx|Generator"
)

juce_generate_juce_header(LFOzTool)

target_sources(LFOzTool
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(LFOzTool
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(LFOzTool
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

cmake_minimum_required(VERSION 3.22)

project(PinkGrain VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version")

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin target
juce_add_plugin(PinkGrain
    COMPANY_NAME "PinkGrain"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
    AU_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
    PLUGIN_MANUFACTURER_CODE Pgra
    PLUGIN_CODE Pgrn
    FORMATS AU VST3
    PRODUCT_NAME "PinkGrain"
)

# Source files
target_sources(PinkGrain
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Grain.cpp
        Source/GrainEngine.cpp
        Source/AudioFileLoader.cpp
        Source/UI/LookAndFeel.cpp
        Source/UI/CustomDial.cpp
        Source/UI/WaveformDisplay.cpp
        Source/UI/LiveWaveformDisplay.cpp
        Source/UI/ZoomedWaveformDisplay.cpp
        Source/UI/VolumeControl.cpp
        Source/UI/ADSRControl.cpp
)

# Generate JuceHeader.h
juce_generate_juce_header(PinkGrain)

# Include directories
target_include_directories(PinkGrain
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# JUCE compile definitions
target_compile_definitions(PinkGrain
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(PinkGrain
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

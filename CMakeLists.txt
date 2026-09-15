cmake_minimum_required(VERSION 3.22)

project(MaFuckinHolyVUMeter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(MaFuckinHolyVUMeter
    COMPANY_NAME "Aidil Audio"
    PRODUCT_NAME "MA Fuckin Holy VU Meter"
    BUNDLE_ID com.aidil.audio.mafuckinholyvumeter
    PLUGIN_MANUFACTURER_CODE Aidl
    PLUGIN_CODE MFVM
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(MaFuckinHolyVUMeter)

target_sources(MaFuckinHolyVUMeter
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h

        Source/DSP/RMSMeter.cpp
        Source/DSP/RMSMeter.h
        Source/DSP/PeakMeter.cpp
        Source/DSP/PeakMeter.h
        Source/DSP/VUMeter.cpp
        Source/DSP/VUMeter.h

        Source/UI/AnalogMeter.cpp
        Source/UI/AnalogMeter.h

        Source/Audio/WasapiLoopbackCapture.cpp
        Source/Audio/WasapiLoopbackCapture.h

        Source/Utils/AudioUtils.h
)

target_include_directories(MaFuckinHolyVUMeter
    PRIVATE
        Source
        Source/Audio
        Source/DSP
        Source/UI
        Source/Utils
)

target_compile_definitions(MaFuckinHolyVUMeter
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MaFuckinHolyVUMeter
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_dsp

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (WIN32)
    target_link_libraries(MaFuckinHolyVUMeter PRIVATE ole32 uuid)
endif()

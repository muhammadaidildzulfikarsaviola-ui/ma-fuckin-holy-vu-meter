cmake_minimum_required(VERSION 3.22)

project(MaFuckinHolyVUMeter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MaFuckinHolyVUMeter
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/StandaloneMain.cpp
    Source/DSP/RMSMeter.cpp
    Source/DSP/PeakMeter.cpp
    Source/DSP/VUMeter.cpp
    Source/UI/AnalogMeter.cpp
    Source/UI/DigitalMeter.cpp
)

target_include_directories(MaFuckinHolyVUMeter PRIVATE
    Source
    Source/DSP
    Source/UI
    Source/Utils
)

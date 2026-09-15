#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Audio/WasapiLoopbackCapture.h"
#include "UI/AnalogMeter.h"

class MaFuckinHolyVUMeterAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit MaFuckinHolyVUMeterAudioProcessorEditor(MaFuckinHolyVUMeterAudioProcessor&);
    ~MaFuckinHolyVUMeterAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshLoopbackDevices();
    void updateMeterSource();
    
    int debugElapsedSeconds = 0;

    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;
    const bool standaloneMode;

    juce::Label titleLabel, subtitleLabel, statusLabel, routingLabel;
    juce::ComboBox sourceModeBox, loopbackDeviceBox;
    juce::TextButton refreshDevicesButton, connectLoopbackButton, resetMetersButton;

    // Top: current true peak (red) + maximum true peak (black).
    AnalogMeter peakLeftMeter  { "L", "TRUE PEAK", true, true };
    AnalogMeter peakRightMeter { "R", "TRUE PEAK", true, true };
    // Bottom: VU needle (black) + peak hold (red).
    AnalogMeter vuLeftMeter    { "L", "VU", true, false };
    AnalogMeter vuRightMeter   { "R", "VU", true, false };

    WasapiLoopbackCapture loopbackCapture;
    std::vector<WasapiLoopbackCapture::DeviceInfo> loopbackDevices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessorEditor)
};

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

    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;
    const bool standaloneMode;

    juce::Label titleLabel, subtitleLabel, statusLabel, routingLabel;
    juce::ComboBox sourceModeBox, loopbackDeviceBox;
    juce::TextButton refreshDevicesButton, connectLoopbackButton;

    // Top row: true-peak meters. Bottom row: VU meters + true-peak hold needle.
    AnalogMeter peakLeftMeter  { "L", "TRUE PEAK", false };
    AnalogMeter peakRightMeter { "R", "TRUE PEAK", false };
    AnalogMeter vuLeftMeter    { "L", "VU", true };
    AnalogMeter vuRightMeter   { "R", "VU", true };

    WasapiLoopbackCapture loopbackCapture;
    std::vector<WasapiLoopbackCapture::DeviceInfo> loopbackDevices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessorEditor)
};

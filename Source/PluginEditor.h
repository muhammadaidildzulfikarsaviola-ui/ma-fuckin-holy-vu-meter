#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Audio/WasapiLoopbackCapture.h"
#include "UI/AnalogMeter.h"

class MaFuckinHolyVUMeterAudioProcessorEditor
    : public juce::AudioProcessorEditor,
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

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label statusLabel;
    juce::Label routingLabel;
    juce::ComboBox sourceModeBox;
    juce::ComboBox loopbackDeviceBox;
    juce::TextButton refreshDevicesButton;
    juce::TextButton connectLoopbackButton;

    AnalogMeter leftMeter { "L", "VU" };
    AnalogMeter rightMeter { "R", "VU" };

    WasapiLoopbackCapture loopbackCapture;
    std::vector<WasapiLoopbackCapture::DeviceInfo> loopbackDevices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessorEditor)
};

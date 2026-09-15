#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Audio/WasapiLoopbackCapture.h"
#include "UI/AnalogMeter.h"
#include "UI/DigitalMeter.h"

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
    enum class DisplayMode { analog, digital };

    void timerCallback() override;
    void setDisplayMode(DisplayMode mode);
    void refreshLoopbackDevices();
    void updateMeterSource();

    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;
    DisplayMode displayMode = DisplayMode::analog;
    const bool standaloneMode;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label leftReadout;
    juce::Label rightReadout;
    juce::Label statusLabel;
    juce::Label routingLabel;
    juce::TextButton analogButton;
    juce::TextButton digitalButton;
    juce::TextButton testSignalButton;
    juce::TextButton refreshDevicesButton;
    juce::TextButton connectLoopbackButton;
    juce::ComboBox sourceModeBox;
    juce::ComboBox loopbackDeviceBox;

    AnalogMeter leftAnalogMeter { "L" };
    AnalogMeter rightAnalogMeter { "R" };
    DigitalMeter digitalMeter;

    WasapiLoopbackCapture loopbackCapture;
    std::vector<WasapiLoopbackCapture::DeviceInfo> loopbackDevices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessorEditor)
};

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
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

    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;
    DisplayMode displayMode = DisplayMode::analog;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label leftReadout;
    juce::Label rightReadout;
    juce::Label statusLabel;
    juce::TextButton analogButton;
    juce::TextButton digitalButton;
    juce::TextButton testSignalButton;
    AnalogMeter leftAnalogMeter { "L" };
    AnalogMeter rightAnalogMeter { "R" };
    DigitalMeter digitalMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessorEditor)
};

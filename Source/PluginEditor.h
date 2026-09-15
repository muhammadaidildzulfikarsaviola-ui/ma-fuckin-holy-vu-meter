#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/AnalogMeter.h"

class MaFuckinHolyVUMeterAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor&);
    ~MaFuckinHolyVUMeterAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label peakLabel;
    juce::Label rmsLabel;
    juce::Label statusLabel;
    juce::TextButton testSignalButton;
    AnalogMeter analogMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        MaFuckinHolyVUMeterAudioProcessorEditor)
};

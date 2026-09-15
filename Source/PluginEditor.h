#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MaFuckinHolyVUMeterAudioProcessorEditor
    : public juce::AudioProcessorEditor
{
public:
    explicit MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor&);
    ~MaFuckinHolyVUMeterAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MaFuckinHolyVUMeterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        MaFuckinHolyVUMeterAudioProcessorEditor)
};

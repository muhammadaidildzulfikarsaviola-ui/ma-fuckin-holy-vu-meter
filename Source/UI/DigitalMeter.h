#pragma once

#include <JuceHeader.h>

class DigitalMeter : public juce::Component
{
public:
    DigitalMeter() = default;
    ~DigitalMeter() override = default;

    void setLevels(float leftPeakDb, float leftRmsDb,
                   float rightPeakDb, float rightRmsDb) noexcept;

    void paint(juce::Graphics& g) override;

private:
    struct ChannelLevels
    {
        float peakDb = -100.0f;
        float rmsDb = -100.0f;
    };

    static constexpr float minDb = -48.0f;
    static constexpr float maxDb = 3.0f;

    ChannelLevels left;
    ChannelLevels right;

    void drawChannel(juce::Graphics& g, juce::Rectangle<float> area,
                     const juce::String& label, const ChannelLevels& levels) const;
    static float normalise(float db) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigitalMeter)
};

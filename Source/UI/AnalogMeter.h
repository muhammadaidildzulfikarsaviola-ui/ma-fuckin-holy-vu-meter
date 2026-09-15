#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    explicit AnalogMeter(juce::String channelLabel = "L");
    ~AnalogMeter() override = default;

    void setLevelDb(float db) noexcept;
    void setPeakDb(float db) noexcept;

    void paint(juce::Graphics& g) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    float levelDb = minDb;
    float peakDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    AnalogMeter(juce::String channelLabel = "L", juce::String meterLabel = "RMS");
    ~AnalogMeter() override = default;

    void setLevelDb(float db) noexcept;
    void setPeakDb(float db) noexcept;

    void paint(juce::Graphics& g) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    juce::String meterType;
    float levelDb = minDb;
    float peakDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

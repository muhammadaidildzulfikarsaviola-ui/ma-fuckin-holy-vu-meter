#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    AnalogMeter(juce::String channelLabel = "L", juce::String meterLabel = "VU");
    ~AnalogMeter() override = default;

    // Both values are expressed relative to the VU scale: 0.0f = 0 VU.
    void setLevelsDb(float vuDb, float peakDb) noexcept;

    void paint(juce::Graphics& g) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    juce::String meterType;
    float vuLevelDb = minDb;
    float peakLevelDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

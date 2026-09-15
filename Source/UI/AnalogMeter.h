#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    AnalogMeter(juce::String channelLabel = "L", juce::String meterLabel = "VU");
    ~AnalogMeter() override = default;

    // Value is expressed relative to the VU scale: 0.0f = 0 VU.
    void setLevelDb(float db) noexcept;

    void paint(juce::Graphics& g) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    juce::String meterType;
    float levelDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    AnalogMeter(juce::String channelLabel = "L",
                juce::String meterLabel = "VU",
                bool showSecondaryNeedle = true,
                bool secondaryIsMax = false);
    ~AnalogMeter() override = default;

    // Main and secondary values are expressed in the displayed dB scale.
    void setLevelsDb(float mainDb, float secondaryDb) noexcept;

    void paint(juce::Graphics&) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    juce::String meterType;
    bool showSecondary = true;
    bool secondaryIsMax = false;
    float mainLevelDb = minDb;
    float secondaryLevelDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

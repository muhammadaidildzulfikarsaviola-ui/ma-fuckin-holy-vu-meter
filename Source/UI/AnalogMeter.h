#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    AnalogMeter(juce::String channelLabel = "L",
                juce::String meterLabel = "VU",
                bool showPeakHoldNeedle = true);
    ~AnalogMeter() override = default;

    // Values are expressed relative to the displayed scale: 0.0f = 0 on the face.
    void setLevelsDb(float mainDb, float peakHoldDb) noexcept;

    void paint(juce::Graphics&) override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    juce::String channel;
    juce::String meterType;
    bool showPeakHold = true;
    float mainLevelDb = minDb;
    float peakHoldLevelDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

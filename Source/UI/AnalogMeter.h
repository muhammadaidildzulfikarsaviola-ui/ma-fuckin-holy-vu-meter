#pragma once

#include <JuceHeader.h>

class AnalogMeter : public juce::Component
{
public:
    void setLevelDb(float db) noexcept;
    float getLevelDb() const noexcept;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    static constexpr float minDb = -20.0f;
    static constexpr float maxDb = 3.0f;

    float levelDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
};

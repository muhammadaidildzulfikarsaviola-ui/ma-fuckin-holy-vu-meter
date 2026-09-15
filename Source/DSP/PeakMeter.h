#pragma once

#include <JuceHeader.h>

class PeakMeter
{
public:
    void prepare();
    void reset();
    void process(const juce::AudioBuffer<float>& buffer);

    float getPeakDb() const noexcept;
    float getPeakLinear() const noexcept;

private:
    std::atomic<float> peakLinear { 0.0f };
};

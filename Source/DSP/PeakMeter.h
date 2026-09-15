#pragma once

#include <JuceHeader.h>
#include <array>

class PeakMeter
{
public:
    static constexpr int channelCount = 2;

    void prepare();
    void reset();
    void process(const juce::AudioBuffer<float>& buffer);
    void processInterleaved(const float* samples, int numFrames, int numChannels) noexcept;
    void processTestSignal(int numSamples, double sampleRate, double& phase);

    float getPeakDb(int channel = 0) const noexcept;
    float getPeakLinear(int channel = 0) const noexcept;

private:
    std::array<std::atomic<float>, channelCount> peakLinear { 0.0f, 0.0f };
};

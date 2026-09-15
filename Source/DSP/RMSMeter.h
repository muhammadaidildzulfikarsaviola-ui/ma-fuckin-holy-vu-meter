#pragma once

#include <JuceHeader.h>
#include <array>

class RMSMeter
{
public:
    static constexpr int channelCount = 2;

    void prepare(double sampleRate);
    void reset();
    void process(const juce::AudioBuffer<float>& buffer);
    void processInterleaved(const float* samples, int numFrames, int numChannels) noexcept;
    void processTestSignal(int numSamples, double sampleRate, double& phase);

    float getRmsDb(int channel = 0) const noexcept;
    float getRmsLinear(int channel = 0) const noexcept;

private:
    std::array<std::atomic<float>, channelCount> rmsLinear { 0.0f, 0.0f };
};

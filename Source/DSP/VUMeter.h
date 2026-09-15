#pragma once

#include <JuceHeader.h>
#include <array>

// Classic VU-style envelope. Uses full-wave average/rectification rather than
// treating RMS itself as the displayed VU value, then applies VU ballistics.
class VUMeter
{
public:
    void prepare(double sampleRate);
    void reset();
    void process(const juce::AudioBuffer<float>& buffer) noexcept;
    void processInterleaved(const float* samples, int numFrames, int numChannels) noexcept;

    float getVuDb(int channel) const noexcept;
    float getVuLinear(int channel) const noexcept;

private:
    static constexpr int channelCount = 2;

    void processAverageLevel(float averageAbs, int channel, double durationSeconds) noexcept;

    double sampleRateHz = 44100.0;
    std::array<float, channelCount> smoothedLinear { 0.0f, 0.0f };
    std::array<std::atomic<float>, channelCount> displayLinear { 0.0f, 0.0f };
};

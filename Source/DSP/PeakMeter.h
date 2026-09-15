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

    void setMaximumPeakFrozen(bool shouldFreeze) noexcept;
    bool isMaximumPeakFrozen() const noexcept;
    void resetMaximumPeak() noexcept;

    float getPeakDb(int channel = 0) const noexcept;
    float getPeakLinear(int channel = 0) const noexcept;
    float getMaximumPeakDb(int channel = 0) const noexcept;
    float getMaximumPeakLinear(int channel = 0) const noexcept;

private:
    void publishPeak(int channel, float value) noexcept;

    std::array<std::atomic<float>, channelCount> peakLinear { 0.0f, 0.0f };
    std::array<std::atomic<float>, channelCount> maximumPeakLinear { 0.0f, 0.0f };
    std::atomic<bool> maximumPeakFrozen { false };
};

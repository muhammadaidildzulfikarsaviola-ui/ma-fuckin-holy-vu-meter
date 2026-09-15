#pragma once

#include <JuceHeader.h>
#include <array>

// Stereo VU display envelope. The audio thread supplies one RMS value per
// channel; the editor reads the smoothed result atomically.
class VUMeter
{
public:
    void prepare(double sampleRate);
    void reset();
    void process(const std::array<float, 2>& rmsLinear, int numSamples) noexcept;

    float getVuDb(int channel) const noexcept;
    float getVuLinear(int channel) const noexcept;

private:
    static constexpr int channelCount = 2;

    double sampleRateHz = 44100.0;
    std::array<float, channelCount> smoothedLinear { 0.0f, 0.0f };
    std::array<std::atomic<float>, channelCount> displayLinear { 0.0f, 0.0f };
};

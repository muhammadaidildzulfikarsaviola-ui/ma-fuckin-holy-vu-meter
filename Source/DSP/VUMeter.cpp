#include "VUMeter.h"

void VUMeter::prepare(double newSampleRate)
{
    sampleRateHz = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void VUMeter::reset()
{
    smoothedLinear.fill(0.0f);

    for (auto& level : displayLinear)
        level.store(0.0f, std::memory_order_relaxed);
}

void VUMeter::process(const std::array<float, 2>& rmsLinear, int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    // Traditional VU movement is deliberately slower than a peak meter.
    constexpr double attackSeconds = 0.300;
    constexpr double releaseSeconds = 1.500;
    const auto blockDuration = static_cast<double>(numSamples) / sampleRateHz;

    for (int channel = 0; channel < channelCount; ++channel)
    {
        const auto target = juce::jmax(0.0f, rmsLinear[static_cast<size_t>(channel)]);
        const auto timeConstant = target > smoothedLinear[static_cast<size_t>(channel)]
            ? attackSeconds : releaseSeconds;
        const auto coefficient = static_cast<float>(std::exp(-blockDuration / timeConstant));

        auto& smoothed = smoothedLinear[static_cast<size_t>(channel)];
        smoothed = target + coefficient * (smoothed - target);
        displayLinear[static_cast<size_t>(channel)].store(smoothed, std::memory_order_relaxed);
    }
}

float VUMeter::getVuDb(int channel) const noexcept
{
    return juce::Decibels::gainToDecibels(getVuLinear(channel), -100.0f);
}

float VUMeter::getVuLinear(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return displayLinear[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

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

void VUMeter::processAverageLevel(float averageAbs, int channel, double durationSeconds) noexcept
{
    // IEC-style VU movement is slow: approximately 300 ms integration to 99%
    // for a step. Release is intentionally a little slower for a hardware feel.
    constexpr double attackSeconds = 0.300;
    constexpr double releaseSeconds = 0.450;
    const auto index = static_cast<size_t>(channel);
    const auto target = juce::jmax(0.0f, averageAbs);
    const auto timeConstant = target >= smoothedLinear[index] ? attackSeconds : releaseSeconds;
    const auto coefficient = static_cast<float>(std::exp(-durationSeconds / timeConstant));

    smoothedLinear[index] = target + coefficient * (smoothedLinear[index] - target);
    displayLinear[index].store(smoothedLinear[index], std::memory_order_relaxed);
}

void VUMeter::process(const juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return;

    const auto duration = static_cast<double>(numSamples) / sampleRateHz;
    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel >= buffer.getNumChannels())
        {
            processAverageLevel(0.0f, channel, duration);
            continue;
        }

        const auto* data = buffer.getReadPointer(channel);
        double sumAbs = 0.0;
        for (int sample = 0; sample < numSamples; ++sample)
            sumAbs += std::abs(static_cast<double>(data[sample]));

        processAverageLevel(static_cast<float>(sumAbs / numSamples), channel, duration);
    }
}

void VUMeter::processInterleaved(const float* samples, int numFrames, int numChannels) noexcept
{
    if (samples == nullptr || numFrames <= 0 || numChannels <= 0)
        return;

    const auto duration = static_cast<double>(numFrames) / sampleRateHz;
    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel >= numChannels)
        {
            processAverageLevel(0.0f, channel, duration);
            continue;
        }

        double sumAbs = 0.0;
        for (int frame = 0; frame < numFrames; ++frame)
            sumAbs += std::abs(static_cast<double>(samples[frame * numChannels + channel]));

        processAverageLevel(static_cast<float>(sumAbs / numFrames), channel, duration);
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

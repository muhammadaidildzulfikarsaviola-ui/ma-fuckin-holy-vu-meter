#include "PeakMeter.h"

void PeakMeter::prepare()
{
    reset();
}

void PeakMeter::reset()
{
    for (auto& peak : peakLinear)
        peak.store(0.0f, std::memory_order_relaxed);

    for (auto& peak : maximumPeakLinear)
        peak.store(0.0f, std::memory_order_relaxed);

    maximumPeakFrozen.store(false, std::memory_order_relaxed);
}

void PeakMeter::process(const juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < channelCount; ++channel)
    {
        const auto value = channel < buffer.getNumChannels()
            ? buffer.getMagnitude(channel, 0, buffer.getNumSamples())
            : 0.0f;
        publishPeak(channel, value);
    }
}

void PeakMeter::processInterleaved(const float* samples, int numFrames, int numChannels) noexcept
{
    for (int channel = 0; channel < channelCount; ++channel)
    {
        float peak = 0.0f;

        if (samples != nullptr && channel < numChannels)
            for (int frame = 0; frame < numFrames; ++frame)
                peak = juce::jmax(peak, std::abs(samples[frame * numChannels + channel]));

        publishPeak(channel, peak);
    }
}

void PeakMeter::processTestSignal(int numSamples, double sampleRate, double& phase)
{
    if (numSamples <= 0 || sampleRate <= 0.0)
        return;

    constexpr double frequency = 440.0;
    constexpr float amplitude = 0.25118864f; // -12 dBFS peak
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
    const auto phaseStep = twoPi * frequency / sampleRate;

    float blockPeak = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = amplitude * static_cast<float>(std::sin(phase));
        blockPeak = juce::jmax(blockPeak, std::abs(value));
        phase += phaseStep;

        if (phase >= twoPi)
            phase -= twoPi;
    }

    for (int channel = 0; channel < channelCount; ++channel)
        publishPeak(channel, blockPeak);
}

void PeakMeter::setMaximumPeakFrozen(bool shouldFreeze) noexcept
{
    maximumPeakFrozen.store(shouldFreeze, std::memory_order_relaxed);
}

bool PeakMeter::isMaximumPeakFrozen() const noexcept
{
    return maximumPeakFrozen.load(std::memory_order_relaxed);
}

void PeakMeter::resetMaximumPeak() noexcept
{
    for (int channel = 0; channel < channelCount; ++channel)
        maximumPeakLinear[static_cast<size_t>(channel)].store(
            peakLinear[static_cast<size_t>(channel)].load(std::memory_order_relaxed),
            std::memory_order_relaxed);
}

float PeakMeter::getPeakDb(int channel) const noexcept
{
    return juce::Decibels::gainToDecibels(getPeakLinear(channel), -100.0f);
}

float PeakMeter::getPeakLinear(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return peakLinear[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

float PeakMeter::getMaximumPeakDb(int channel) const noexcept
{
    return juce::Decibels::gainToDecibels(getMaximumPeakLinear(channel), -100.0f);
}

float PeakMeter::getMaximumPeakLinear(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return maximumPeakLinear[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

void PeakMeter::publishPeak(int channel, float value) noexcept
{
    const auto index = static_cast<size_t>(channel);
    peakLinear[index].store(value, std::memory_order_relaxed);

    if (maximumPeakFrozen.load(std::memory_order_relaxed))
        return;

    auto previous = maximumPeakLinear[index].load(std::memory_order_relaxed);
    while (value > previous
           && !maximumPeakLinear[index].compare_exchange_weak(
               previous, value, std::memory_order_relaxed, std::memory_order_relaxed))
    {
    }
}

#include "RMSMeter.h"

void RMSMeter::prepare(double sampleRate)
{
    juce::ignoreUnused(sampleRate);
    reset();
}

void RMSMeter::reset()
{
    for (auto& rms : rmsLinear)
        rms.store(0.0f, std::memory_order_relaxed);
}

void RMSMeter::process(const juce::AudioBuffer<float>& buffer)
{
    const auto sampleCount = buffer.getNumSamples();

    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel >= buffer.getNumChannels() || sampleCount <= 0)
        {
            rmsLinear[static_cast<size_t>(channel)].store(0.0f, std::memory_order_relaxed);
            continue;
        }

        const auto* data = buffer.getReadPointer(channel);
        double sumSquares = 0.0;

        for (int sample = 0; sample < sampleCount; ++sample)
        {
            const auto value = static_cast<double>(data[sample]);
            sumSquares += value * value;
        }

        const auto rms = static_cast<float>(std::sqrt(sumSquares / sampleCount));
        rmsLinear[static_cast<size_t>(channel)].store(rms, std::memory_order_relaxed);
    }
}

void RMSMeter::processTestSignal(int numSamples, double sampleRate, double& phase)
{
    if (numSamples <= 0 || sampleRate <= 0.0)
        return;

    constexpr double frequency = 440.0;
    constexpr float amplitude = 0.25118864f; // -12 dBFS peak
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
    const auto phaseStep = twoPi * frequency / sampleRate;

    double sumSquares = 0.0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = amplitude * static_cast<float>(std::sin(phase));
        sumSquares += static_cast<double>(value) * value;
        phase += phaseStep;

        if (phase >= twoPi)
            phase -= twoPi;
    }

    const auto rms = static_cast<float>(std::sqrt(sumSquares / numSamples));

    for (auto& level : rmsLinear)
        level.store(rms, std::memory_order_relaxed);
}

float RMSMeter::getRmsDb(int channel) const noexcept
{
    return juce::Decibels::gainToDecibels(getRmsLinear(channel), -100.0f);
}

float RMSMeter::getRmsLinear(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return rmsLinear[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

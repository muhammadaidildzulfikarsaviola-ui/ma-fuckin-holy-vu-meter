#include "RMSMeter.h"

void RMSMeter::prepare(double sampleRate)
{
    juce::ignoreUnused(sampleRate);
    reset();
}

void RMSMeter::reset()
{
    rmsLinear.store(0.0f, std::memory_order_relaxed);
}

void RMSMeter::process(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    double sumSquares = 0.0;
    int sampleCount = 0;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* data = buffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const double value = static_cast<double>(data[sample]);
            sumSquares += value * value;
            ++sampleCount;
        }
    }

    if (sampleCount > 0)
    {
        const auto rms = static_cast<float>(std::sqrt(sumSquares / sampleCount));
        rmsLinear.store(rms, std::memory_order_relaxed);
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
    rmsLinear.store(rms, std::memory_order_relaxed);
}

float RMSMeter::getRmsDb() const noexcept
{
    return juce::Decibels::gainToDecibels(getRmsLinear(), -100.0f);
}

float RMSMeter::getRmsLinear() const noexcept
{
    return rmsLinear.load(std::memory_order_relaxed);
}

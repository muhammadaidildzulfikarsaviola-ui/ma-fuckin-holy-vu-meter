#include "PeakMeter.h"

void PeakMeter::prepare()
{
    reset();
}

void PeakMeter::reset()
{
    peakLinear.store(0.0f, std::memory_order_relaxed);
}

void PeakMeter::process(const juce::AudioBuffer<float>& buffer)
{
    float blockPeak = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        blockPeak = juce::jmax(blockPeak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));

    peakLinear.store(blockPeak, std::memory_order_relaxed);
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

    peakLinear.store(blockPeak, std::memory_order_relaxed);
}

float PeakMeter::getPeakDb() const noexcept
{
    return juce::Decibels::gainToDecibels(getPeakLinear(), -100.0f);
}

float PeakMeter::getPeakLinear() const noexcept
{
    return peakLinear.load(std::memory_order_relaxed);
}

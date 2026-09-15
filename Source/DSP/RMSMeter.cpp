#include "RMSMeter.h"

void RMSMeter::prepare(double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
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

float RMSMeter::getRmsDb() const noexcept
{
    return juce::Decibels::gainToDecibels(getRmsLinear(), -100.0f);
}

float RMSMeter::getRmsLinear() const noexcept
{
    return rmsLinear.load(std::memory_order_relaxed);
}

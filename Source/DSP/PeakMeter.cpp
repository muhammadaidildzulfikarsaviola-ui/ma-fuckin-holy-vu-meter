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

float PeakMeter::getPeakDb() const noexcept
{
    return juce::Decibels::gainToDecibels(getPeakLinear(), -100.0f);
}

float PeakMeter::getPeakLinear() const noexcept
{
    return peakLinear.load(std::memory_order_relaxed);
}

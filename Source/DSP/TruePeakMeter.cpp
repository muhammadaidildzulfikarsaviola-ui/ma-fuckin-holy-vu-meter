#include "TruePeakMeter.h"

void TruePeakMeter::prepare(double newSampleRate, int maxBlockSize)
{
    sampleRateHz = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    // 4x oversampling catches inter-sample peaks that a sample peak meter misses.
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        channelCount,
        2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true,
        false);

    oversampler->initProcessing(static_cast<size_t>(juce::jmax(1, maxBlockSize)));
    oversampler->reset();
    loopbackBuffer.setSize(channelCount, juce::jmax(1, maxBlockSize), false, false, true);
    reset();
}

void TruePeakMeter::reset()
{
    for (auto& peak : currentPeakDb)
        peak.store(-100.0f, std::memory_order_relaxed);
    for (auto& peak : maxPeakDb)
        peak.store(-100.0f, std::memory_order_relaxed);
}

void TruePeakMeter::process(const juce::AudioBuffer<float>& buffer) noexcept
{
    if (oversampler == nullptr || buffer.getNumSamples() <= 0)
        return;

    const auto samples = buffer.getNumSamples();
    if (loopbackBuffer.getNumSamples() < samples)
        loopbackBuffer.setSize(channelCount, samples, false, false, true);

    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel < buffer.getNumChannels())
            loopbackBuffer.copyFrom(channel, 0, buffer, channel, 0, samples);
        else
            loopbackBuffer.clear(channel, 0, samples);
    }

    juce::dsp::AudioBlock<float> block(loopbackBuffer);
    auto upsampled = oversampler->processSamplesUp(block);
    processOversampledBlock(upsampled);
    oversampler->processSamplesDown(block);
}

void TruePeakMeter::processInterleaved(const float* interleavedSamples,
                                       int numFrames,
                                       int numChannels) noexcept
{
    if (oversampler == nullptr || interleavedSamples == nullptr
        || numFrames <= 0 || numChannels <= 0)
        return;

    if (loopbackBuffer.getNumSamples() < numFrames)
        loopbackBuffer.setSize(channelCount, numFrames, false, false, true);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        for (int channel = 0; channel < channelCount; ++channel)
        {
            const auto sourceChannel = juce::jmin(channel, numChannels - 1);
            loopbackBuffer.setSample(channel, frame,
                                     interleavedSamples[frame * numChannels + sourceChannel]);
        }
    }

    juce::dsp::AudioBlock<float> block(loopbackBuffer.getArrayOfWritePointers(),
                                       channelCount,
                                       static_cast<size_t>(numFrames));
    auto upsampled = oversampler->processSamplesUp(block);
    processOversampledBlock(upsampled);
    oversampler->processSamplesDown(block);
}

void TruePeakMeter::processOversampledBlock(const juce::dsp::AudioBlock<float>& block) noexcept
{
    for (int channel = 0; channel < channelCount; ++channel)
    {
        const auto* data = block.getChannelPointer(static_cast<size_t>(channel));
        const auto numSamples = block.getNumSamples();
        float peak = 0.0f;

        for (size_t sample = 0; sample < numSamples; ++sample)
            peak = juce::jmax(peak, std::abs(data[sample]));

        const auto db = juce::Decibels::gainToDecibels(peak, -100.0f);

        // Current peak is the peak of the latest processed audio block.
        // It is intentionally NOT accumulated, otherwise the needle can get stuck.
        currentPeakDb[static_cast<size_t>(channel)].store(db, std::memory_order_relaxed);

        // Maximum true peak is retained until the processor is reset.
        auto currentMax = maxPeakDb[static_cast<size_t>(channel)].load(std::memory_order_relaxed);
        while (db > currentMax
               && !maxPeakDb[static_cast<size_t>(channel)].compare_exchange_weak(
                   currentMax, db, std::memory_order_relaxed))
        {
        }
    }
}

float TruePeakMeter::getCurrentPeakDb(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return currentPeakDb[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

float TruePeakMeter::getMaxPeakDb(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return maxPeakDb[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

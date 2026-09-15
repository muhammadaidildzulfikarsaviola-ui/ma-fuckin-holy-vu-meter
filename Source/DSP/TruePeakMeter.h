#pragma once

#include <JuceHeader.h>
#include <array>

class TruePeakMeter
{
public:
    static constexpr int channelCount = 2;

    TruePeakMeter() = default;
    ~TruePeakMeter() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void process(const juce::AudioBuffer<float>& buffer) noexcept;
    void processInterleaved(const float* interleavedSamples,
                            int numFrames,
                            int numChannels) noexcept;

    float getCurrentPeakDb(int channel = 0) const noexcept;
    float getMaxPeakDb(int channel = 0) const noexcept;

private:
    void processOversampledBlock(const juce::dsp::AudioBlock<float>& block) noexcept;

    double sampleRateHz = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::AudioBuffer<float> loopbackBuffer;
    std::array<std::atomic<float>, channelCount> currentPeakDb { -100.0f, -100.0f };
    std::array<std::atomic<float>, channelCount> maxPeakDb { -100.0f, -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruePeakMeter)
};

#pragma once

#include <JuceHeader.h>
#include "DSP/VUMeter.h"
#include "DSP/TruePeakMeter.h"

class MaFuckinHolyVUMeterAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int channelCount = 2;
    enum class MeterSource : int { normalInput, wasapiLoopback };

    MaFuckinHolyVUMeterAudioProcessor();
    ~MaFuckinHolyVUMeterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processLoopbackSamples(const float* interleavedSamples, int numFrames, int numChannels, double sourceSampleRate) noexcept;
    void setMeterSource(MeterSource source) noexcept;
    MeterSource getMeterSource() const noexcept;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    float getVuDb(int channel = 0) const noexcept;
    float getTruePeakCurrentDb(int channel = 0) const noexcept;
    float getTruePeakMaxDb(int channel = 0) const noexcept;
    float getVuPeakHoldDb(int channel = 0) const noexcept;

private:
    void updateVuPeakHold(int numSamples) noexcept;

    VUMeter vuMeter;
    TruePeakMeter truePeakMeter;

    std::array<std::atomic<float>, channelCount> vuPeakHoldDb { -100.0f, -100.0f };
    std::array<float, channelCount> vuPeakHoldElapsedSeconds { 0.0f, 0.0f };
    std::atomic<MeterSource> meterSource { MeterSource::normalInput };
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

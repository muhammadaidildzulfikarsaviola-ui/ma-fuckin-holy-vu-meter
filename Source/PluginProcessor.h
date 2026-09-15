#pragma once

#include <JuceHeader.h>
#include "DSP/PeakMeter.h"
#include "DSP/RMSMeter.h"
#include "DSP/VUMeter.h"

class MaFuckinHolyVUMeterAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int channelCount = 2;

    enum class MeterSource : int
    {
        normalInput,
        wasapiLoopback
    };

    MaFuckinHolyVUMeterAudioProcessor();
    ~MaFuckinHolyVUMeterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Called only by the Windows standalone loopback capture thread. This
    // updates meter state only; it never writes to a JUCE/host audio buffer.
    void processLoopbackSamples(const float* interleavedSamples, int numFrames,
                                int numChannels, double sourceSampleRate) noexcept;

    void setMeterSource(MeterSource source) noexcept;
    MeterSource getMeterSource() const noexcept;

    void setMaximumPeakFrozen(bool shouldFreeze) noexcept;
    bool isMaximumPeakFrozen() const noexcept;
    void resetMaximumPeak() noexcept;

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

    float getPeakDb(int channel = 0) const noexcept;
    float getRmsDb(int channel = 0) const noexcept;
    float getVuDb(int channel = 0) const noexcept;
    float getMaximumPeakDb(int channel = 0) const noexcept;

    void setTestSignalEnabled(bool enabled) noexcept;
    bool isTestSignalEnabled() const noexcept;

private:
    void updateVuMeter(int numSamples) noexcept;

    PeakMeter peakMeter;
    RMSMeter rmsMeter;
    VUMeter vuMeter;

    std::atomic<MeterSource> meterSource { MeterSource::normalInput };
    std::atomic<bool> testSignalEnabled { false };
    double sampleRate = 44100.0;
    double testSignalPhase = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

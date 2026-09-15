#pragma once

#include <JuceHeader.h>
#include "DSP/PeakMeter.h"
#include "DSP/RMSMeter.h"
#include "DSP/VUMeter.h"

class MaFuckinHolyVUMeterAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int channelCount = 2;

    MaFuckinHolyVUMeterAudioProcessor();
    ~MaFuckinHolyVUMeterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    void setTestSignalEnabled(bool enabled) noexcept;
    bool isTestSignalEnabled() const noexcept;

private:
    PeakMeter peakMeter;
    RMSMeter rmsMeter;
    VUMeter vuMeter;

    std::atomic<bool> testSignalEnabled { false };
    double sampleRate = 44100.0;
    double testSignalPhase = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaFuckinHolyVUMeterAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

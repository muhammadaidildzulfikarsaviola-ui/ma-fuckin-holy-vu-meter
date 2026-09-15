#include "PluginProcessor.h"
#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessor::MaFuckinHolyVUMeterAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

MaFuckinHolyVUMeterAudioProcessor::~MaFuckinHolyVUMeterAudioProcessor() = default;

const juce::String MaFuckinHolyVUMeterAudioProcessor::getName() const { return JucePlugin_Name; }

void MaFuckinHolyVUMeterAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    vuMeter.prepare(sampleRate);
    truePeakMeter.prepare(sampleRate, samplesPerBlock);

    for (auto& peak : vuPeakHoldDb)
        peak.store(-100.0f, std::memory_order_relaxed);
    vuPeakHoldElapsedSeconds.fill(0.0f);
}

void MaFuckinHolyVUMeterAudioProcessor::releaseResources() {}

bool MaFuckinHolyVUMeterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MaFuckinHolyVUMeterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (getMeterSource() == MeterSource::wasapiLoopback)
        return;

    vuMeter.process(buffer);
    truePeakMeter.process(buffer);
    updateVuPeakHold(buffer.getNumSamples());
}

void MaFuckinHolyVUMeterAudioProcessor::processLoopbackSamples(
    const float* interleavedSamples, int numFrames, int numChannels, double sourceSampleRate) noexcept
{
    if (getMeterSource() != MeterSource::wasapiLoopback)
        return;

    if (sourceSampleRate > 0.0)
    {
        sampleRate = sourceSampleRate;
        vuMeter.prepare(sampleRate);
        truePeakMeter.prepare(sampleRate, numFrames);
    }

    vuMeter.processInterleaved(interleavedSamples, numFrames, numChannels);
    truePeakMeter.processInterleaved(interleavedSamples, numFrames, numChannels);
    updateVuPeakHold(numFrames);
}

void MaFuckinHolyVUMeterAudioProcessor::setMeterSource(MeterSource source) noexcept
{
    meterSource.store(source, std::memory_order_relaxed);
}

MaFuckinHolyVUMeterAudioProcessor::MeterSource MaFuckinHolyVUMeterAudioProcessor::getMeterSource() const noexcept
{
    return meterSource.load(std::memory_order_relaxed);
}

void MaFuckinHolyVUMeterAudioProcessor::updateVuPeakHold(int numSamples) noexcept
{
    if (numSamples <= 0 || sampleRate <= 0.0)
        return;

    constexpr float holdSeconds = 1.0f;
    constexpr float decayDbPerSecond = 3.0f;
    const auto blockSeconds = static_cast<float>(numSamples / sampleRate);

    for (int channel = 0; channel < channelCount; ++channel)
    {
        // This hold belongs to the VU meter, so it follows the VU envelope
        // rather than the independent true-peak detector.
        const auto currentVuDb = getVuDb(channel);
        auto heldDb = vuPeakHoldDb[static_cast<size_t>(channel)].load(std::memory_order_relaxed);

        if (currentVuDb > heldDb)
        {
            heldDb = currentVuDb;
            vuPeakHoldElapsedSeconds[static_cast<size_t>(channel)] = 0.0f;
        }
        else
        {
            auto& elapsed = vuPeakHoldElapsedSeconds[static_cast<size_t>(channel)];
            elapsed += blockSeconds;
            if (elapsed > holdSeconds)
                heldDb -= decayDbPerSecond * blockSeconds;
        }

        vuPeakHoldDb[static_cast<size_t>(channel)].store(
            juce::jmax(-100.0f, heldDb), std::memory_order_relaxed);
    }
}

float MaFuckinHolyVUMeterAudioProcessor::getVuDb(int channel) const noexcept
{
    return vuMeter.getVuDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getTruePeakCurrentDb(int channel) const noexcept
{
    return truePeakMeter.getCurrentPeakDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getTruePeakMaxDb(int channel) const noexcept
{
    return truePeakMeter.getMaxPeakDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getVuPeakHoldDb(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return vuPeakHoldDb[static_cast<size_t>(index)].load(std::memory_order_relaxed);
}

bool MaFuckinHolyVUMeterAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* MaFuckinHolyVUMeterAudioProcessor::createEditor() { return new MaFuckinHolyVUMeterAudioProcessorEditor(*this); }
bool MaFuckinHolyVUMeterAudioProcessor::acceptsMidi() const { return false; }
bool MaFuckinHolyVUMeterAudioProcessor::producesMidi() const { return false; }
bool MaFuckinHolyVUMeterAudioProcessor::isMidiEffect() const { return false; }
double MaFuckinHolyVUMeterAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MaFuckinHolyVUMeterAudioProcessor::getNumPrograms() { return 1; }
int MaFuckinHolyVUMeterAudioProcessor::getCurrentProgram() { return 0; }
void MaFuckinHolyVUMeterAudioProcessor::setCurrentProgram(int) {}
const juce::String MaFuckinHolyVUMeterAudioProcessor::getProgramName(int) { return {}; }
void MaFuckinHolyVUMeterAudioProcessor::changeProgramName(int, const juce::String&) {}
void MaFuckinHolyVUMeterAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void MaFuckinHolyVUMeterAudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaFuckinHolyVUMeterAudioProcessor();
}

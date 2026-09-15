#include "PluginProcessor.h"
#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessor::MaFuckinHolyVUMeterAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

MaFuckinHolyVUMeterAudioProcessor::~MaFuckinHolyVUMeterAudioProcessor() = default;

const juce::String MaFuckinHolyVUMeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void MaFuckinHolyVUMeterAudioProcessor::prepareToPlay(double newSampleRate, int)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    rmsMeter.prepare(sampleRate);
    vuMeter.prepare(sampleRate);
}

void MaFuckinHolyVUMeterAudioProcessor::releaseResources()
{
}

bool MaFuckinHolyVUMeterAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    if (output != juce::AudioChannelSet::stereo())
        return false;

    return input == juce::AudioChannelSet::stereo();
}

void MaFuckinHolyVUMeterAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // In standalone loopback mode, native WASAPI capture owns the meter input.
    // The host callback remains a transparent passthrough.
    if (getMeterSource() == MeterSource::wasapiLoopback)
        return;

    rmsMeter.process(buffer);
    updateVuMeter(buffer.getNumSamples());
}

void MaFuckinHolyVUMeterAudioProcessor::processLoopbackSamples(
    const float* interleavedSamples, int numFrames, int numChannels,
    double sourceSampleRate) noexcept
{
    juce::ignoreUnused(sourceSampleRate);

    if (getMeterSource() != MeterSource::wasapiLoopback)
        return;

    rmsMeter.processInterleaved(interleavedSamples, numFrames, numChannels);
    updateVuMeter(numFrames);
}

void MaFuckinHolyVUMeterAudioProcessor::setMeterSource(MeterSource source) noexcept
{
    meterSource.store(source, std::memory_order_relaxed);
}

MaFuckinHolyVUMeterAudioProcessor::MeterSource
MaFuckinHolyVUMeterAudioProcessor::getMeterSource() const noexcept
{
    return meterSource.load(std::memory_order_relaxed);
}

void MaFuckinHolyVUMeterAudioProcessor::updateVuMeter(int numSamples) noexcept
{
    std::array<float, channelCount> rmsLevels {};
    for (int channel = 0; channel < channelCount; ++channel)
        rmsLevels[static_cast<size_t>(channel)] = rmsMeter.getRmsLinear(channel);

    vuMeter.process(rmsLevels, numSamples);
}

float MaFuckinHolyVUMeterAudioProcessor::getRmsDb(int channel) const noexcept
{
    return rmsMeter.getRmsDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getVuDb(int channel) const noexcept
{
    return vuMeter.getVuDb(channel);
}

bool MaFuckinHolyVUMeterAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor*
MaFuckinHolyVUMeterAudioProcessor::createEditor()
{
    return new MaFuckinHolyVUMeterAudioProcessorEditor(*this);
}

bool MaFuckinHolyVUMeterAudioProcessor::acceptsMidi() const { return false; }
bool MaFuckinHolyVUMeterAudioProcessor::producesMidi() const { return false; }
bool MaFuckinHolyVUMeterAudioProcessor::isMidiEffect() const { return false; }
double MaFuckinHolyVUMeterAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MaFuckinHolyVUMeterAudioProcessor::getNumPrograms() { return 1; }
int MaFuckinHolyVUMeterAudioProcessor::getCurrentProgram() { return {}; }
void MaFuckinHolyVUMeterAudioProcessor::setCurrentProgram(int) {}
const juce::String MaFuckinHolyVUMeterAudioProcessor::getProgramName(int) { return {}; }
void MaFuckinHolyVUMeterAudioProcessor::changeProgramName(int, const juce::String&) {}
void MaFuckinHolyVUMeterAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void MaFuckinHolyVUMeterAudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaFuckinHolyVUMeterAudioProcessor();
}

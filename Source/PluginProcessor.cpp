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
    testSignalPhase = 0.0;

    peakMeter.prepare();
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

    if (testSignalEnabled.load(std::memory_order_relaxed))
    {
        // Diagnostic signal goes only to the meters; the host buffer is never
        // altered, keeping both VST3 and standalone processing transparent.
        auto rmsPhase = testSignalPhase;
        peakMeter.processTestSignal(buffer.getNumSamples(), sampleRate, testSignalPhase);
        rmsMeter.processTestSignal(buffer.getNumSamples(), sampleRate, rmsPhase);
    }
    else
    {
        peakMeter.process(buffer);
        rmsMeter.process(buffer);
    }

    std::array<float, channelCount> rmsLevels {};
    for (int channel = 0; channel < channelCount; ++channel)
        rmsLevels[static_cast<size_t>(channel)] = rmsMeter.getRmsLinear(channel);

    vuMeter.process(rmsLevels, buffer.getNumSamples());
}

float MaFuckinHolyVUMeterAudioProcessor::getPeakDb(int channel) const noexcept
{
    return peakMeter.getPeakDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getRmsDb(int channel) const noexcept
{
    return rmsMeter.getRmsDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getVuDb(int channel) const noexcept
{
    return vuMeter.getVuDb(channel);
}

void MaFuckinHolyVUMeterAudioProcessor::setTestSignalEnabled(bool enabled) noexcept
{
    testSignalEnabled.store(enabled, std::memory_order_relaxed);
}

bool MaFuckinHolyVUMeterAudioProcessor::isTestSignalEnabled() const noexcept
{
    return testSignalEnabled.load(std::memory_order_relaxed);
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

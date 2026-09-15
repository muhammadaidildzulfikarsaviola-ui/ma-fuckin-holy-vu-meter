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
        // Temporary diagnostic mode. The generated 440 Hz signal is fed only
        // into the meters. The real audio buffer remains untouched.
        peakMeter.processTestSignal(buffer.getNumSamples(), sampleRate, testSignalPhase);
        rmsMeter.processTestSignal(buffer.getNumSamples(), sampleRate, testSignalPhase);
    }
    else
    {
        peakMeter.process(buffer);
        rmsMeter.process(buffer);
    }

    // Meter only. Keep the real audio path untouched so the plugin remains a
    // transparent passthrough when inserted into a DAW channel.
}

float MaFuckinHolyVUMeterAudioProcessor::getPeakDb() const noexcept
{
    return peakMeter.getPeakDb();
}

float MaFuckinHolyVUMeterAudioProcessor::getRmsDb() const noexcept
{
    return rmsMeter.getRmsDb();
}

void MaFuckinHolyVUMeterAudioProcessor::setTestSignalEnabled(bool enabled) noexcept
{
    testSignalEnabled.store(enabled, std::memory_order_relaxed);

    if (enabled)
        testSignalPhase = 0.0;
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

bool MaFuckinHolyVUMeterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool MaFuckinHolyVUMeterAudioProcessor::producesMidi() const
{
    return false;
}

bool MaFuckinHolyVUMeterAudioProcessor::isMidiEffect() const
{
    return false;
}

double MaFuckinHolyVUMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MaFuckinHolyVUMeterAudioProcessor::getNumPrograms()
{
    return 1;
}

int MaFuckinHolyVUMeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MaFuckinHolyVUMeterAudioProcessor::setCurrentProgram(int)
{
}

const juce::String MaFuckinHolyVUMeterAudioProcessor::getProgramName(int)
{
    return {};
}

void MaFuckinHolyVUMeterAudioProcessor::changeProgramName(
    int,
    const juce::String&)
{
}

void MaFuckinHolyVUMeterAudioProcessor::getStateInformation(
    juce::MemoryBlock&)
{
}

void MaFuckinHolyVUMeterAudioProcessor::setStateInformation(
    const void*,
    int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaFuckinHolyVUMeterAudioProcessor();
}

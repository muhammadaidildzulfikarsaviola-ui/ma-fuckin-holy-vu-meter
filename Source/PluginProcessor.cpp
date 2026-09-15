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

void MaFuckinHolyVUMeterAudioProcessor::prepareToPlay(double, int)
{
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

    // Temporary passthrough.
    // Meter DSP will be connected here in the next milestone.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            data[sample] = data[sample];
    }
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

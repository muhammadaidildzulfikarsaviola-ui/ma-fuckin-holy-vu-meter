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

    for (auto& peak : peakHoldDb)
        peak.store(-100.0f, std::memory_order_relaxed);

    peakHoldElapsedSeconds.fill(0.0f);
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
    updatePeakMeter(buffer);
}

void MaFuckinHolyVUMeterAudioProcessor::processLoopbackSamples(
    const float* interleavedSamples, int numFrames, int numChannels,
    double sourceSampleRate) noexcept
{
    if (getMeterSource() != MeterSource::wasapiLoopback)
        return;

    rmsMeter.processInterleaved(interleavedSamples, numFrames, numChannels);
    updateVuMeter(numFrames);
    updatePeakMeterInterleaved(interleavedSamples, numFrames, numChannels);

    // Keep the loopback detector calibrated to the source stream's sample rate.
    if (sourceSampleRate > 0.0)
        sampleRate = sourceSampleRate;
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

void MaFuckinHolyVUMeterAudioProcessor::updatePeakMeter(
    const juce::AudioBuffer<float>& buffer) noexcept
{
    const auto frames = buffer.getNumSamples();
    if (frames <= 0)
        return;

    // Peak needle: instantaneous block peak, then a short hold and slow return.
    constexpr float holdSeconds = 1.0f;
    constexpr float decayDbPerSecond = 3.0f;
    const auto blockSeconds = static_cast<float>(frames / sampleRate);

    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel >= buffer.getNumChannels())
            continue;

        const auto* data = buffer.getReadPointer(channel);
        float blockPeak = 0.0f;

        for (int sample = 0; sample < frames; ++sample)
            blockPeak = juce::jmax(blockPeak, std::abs(data[sample]));

        const auto peakDb = juce::Decibels::gainToDecibels(blockPeak, -100.0f);
        auto heldDb = peakHoldDb[static_cast<size_t>(channel)].load(std::memory_order_relaxed);

        if (peakDb > heldDb)
        {
            heldDb = peakDb;
            peakHoldElapsedSeconds[static_cast<size_t>(channel)] = 0.0f;
        }
        else
        {
            auto& elapsed = peakHoldElapsedSeconds[static_cast<size_t>(channel)];
            elapsed += blockSeconds;

            if (elapsed > holdSeconds)
                heldDb -= decayDbPerSecond * blockSeconds;
        }

        heldDb = juce::jmax(-100.0f, heldDb);
        peakHoldDb[static_cast<size_t>(channel)].store(heldDb, std::memory_order_relaxed);
    }
}

void MaFuckinHolyVUMeterAudioProcessor::updatePeakMeterInterleaved(
    const float* interleavedSamples, int numFrames, int numChannels) noexcept
{
    if (interleavedSamples == nullptr || numFrames <= 0 || numChannels <= 0)
        return;

    constexpr float holdSeconds = 1.0f;
    constexpr float decayDbPerSecond = 3.0f;
    const auto blockSeconds = static_cast<float>(numFrames / sampleRate);

    for (int channel = 0; channel < channelCount; ++channel)
    {
        if (channel >= numChannels)
            continue;

        float blockPeak = 0.0f;
        for (int frame = 0; frame < numFrames; ++frame)
        {
            const auto sample = interleavedSamples[frame * numChannels + channel];
            blockPeak = juce::jmax(blockPeak, std::abs(sample));
        }

        const auto peakDb = juce::Decibels::gainToDecibels(blockPeak, -100.0f);
        auto heldDb = peakHoldDb[static_cast<size_t>(channel)].load(std::memory_order_relaxed);

        if (peakDb > heldDb)
        {
            heldDb = peakDb;
            peakHoldElapsedSeconds[static_cast<size_t>(channel)] = 0.0f;
        }
        else
        {
            auto& elapsed = peakHoldElapsedSeconds[static_cast<size_t>(channel)];
            elapsed += blockSeconds;

            if (elapsed > holdSeconds)
                heldDb -= decayDbPerSecond * blockSeconds;
        }

        heldDb = juce::jmax(-100.0f, heldDb);
        peakHoldDb[static_cast<size_t>(channel)].store(heldDb, std::memory_order_relaxed);
    }
}

float MaFuckinHolyVUMeterAudioProcessor::getRmsDb(int channel) const noexcept
{
    return rmsMeter.getRmsDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getVuDb(int channel) const noexcept
{
    return vuMeter.getVuDb(channel);
}

float MaFuckinHolyVUMeterAudioProcessor::getPeakDb(int channel) const noexcept
{
    const auto index = juce::jlimit(0, channelCount - 1, channel);
    return peakHoldDb[static_cast<size_t>(index)].load(std::memory_order_relaxed);
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

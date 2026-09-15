#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessorEditor::
    MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(800, 500);
}

MaFuckinHolyVUMeterAudioProcessorEditor::
    ~MaFuckinHolyVUMeterAudioProcessorEditor() = default;

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);

    g.drawText(
        "MA FUCKIN HOLY VU METER",
        getLocalBounds(),
        juce::Justification::centred,
        true);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
}

#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessorEditor::
    MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    statusLabel.setText("JUCE AUDIO ENGINE READY", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    setSize(800, 500);
}

MaFuckinHolyVUMeterAudioProcessorEditor::
    ~MaFuckinHolyVUMeterAudioProcessorEditor() = default;

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    titleLabel.setBounds(area.removeFromTop(70).reduced(20, 10));
    statusLabel.setBounds(area.removeFromTop(40).reduced(20, 5));
}

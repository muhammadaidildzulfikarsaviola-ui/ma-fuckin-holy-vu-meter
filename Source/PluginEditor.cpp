#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessorEditor::
    MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      levelBar(displayedLevel)
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(24.0f).withStyle("Bold")));
    addAndMakeVisible(titleLabel);

    peakLabel.setText("PEAK: -100.0 dB", juce::dontSendNotification);
    peakLabel.setJustificationType(juce::Justification::centred);
    peakLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(22.0f)));
    addAndMakeVisible(peakLabel);

    rmsLabel.setText("RMS: -100.0 dB", juce::dontSendNotification);
    rmsLabel.setJustificationType(juce::Justification::centred);
    rmsLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(22.0f)));
    addAndMakeVisible(rmsLabel);

    statusLabel.setText("AUDIO CALLBACK READY", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    levelBar.setPercentageDisplay(false);
    addAndMakeVisible(levelBar);

    setSize(800, 500);
    startTimerHz(30);
}

MaFuckinHolyVUMeterAudioProcessorEditor::
    ~MaFuckinHolyVUMeterAudioProcessorEditor()
{
    stopTimer();
}

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto meterArea = getLocalBounds().reduced(30);
    meterArea.removeFromTop(150);
    g.setColour(juce::Colours::darkgrey);
    g.drawRoundedRectangle(meterArea.toFloat(), 8.0f, 2.0f);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(30);

    titleLabel.setBounds(area.removeFromTop(60));
    peakLabel.setBounds(area.removeFromTop(40));
    rmsLabel.setBounds(area.removeFromTop(40));
    levelBar.setBounds(area.removeFromTop(45).reduced(20, 5));
    statusLabel.setBounds(area.removeFromBottom(35));
}

void MaFuckinHolyVUMeterAudioProcessorEditor::timerCallback()
{
    const auto peakDb = audioProcessor.getPeakDb();
    const auto rmsDb = audioProcessor.getRmsDb();

    peakLabel.setText(
        "PEAK: " + juce::String(peakDb, 1) + " dB",
        juce::dontSendNotification);

    rmsLabel.setText(
        "RMS: " + juce::String(rmsDb, 1) + " dB",
        juce::dontSendNotification);

    const auto normalized = juce::jlimit(0.0f, 1.0f, juce::jmap(peakDb, -60.0f, 0.0f, 0.0f, 1.0f));
    displayedLevel = normalized;
    levelBar.setValue(displayedLevel, juce::dontSendNotification);

    statusLabel.setText(
        peakDb > -60.0f ? "AUDIO SIGNAL DETECTED" : "WAITING FOR AUDIO SIGNAL",
        juce::dontSendNotification);
}

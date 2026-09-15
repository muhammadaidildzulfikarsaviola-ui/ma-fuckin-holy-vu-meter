#include "PluginEditor.h"

MaFuckinHolyVUMeterAudioProcessorEditor::
    MaFuckinHolyVUMeterAudioProcessorEditor(
        MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(24.0f).withStyle("Bold")));
    addAndMakeVisible(titleLabel);

    peakLabel.setText("PEAK: -100.0 dBFS", juce::dontSendNotification);
    peakLabel.setJustificationType(juce::Justification::centred);
    peakLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(18.0f)));
    addAndMakeVisible(peakLabel);

    rmsLabel.setText("RMS: -100.0 dBFS", juce::dontSendNotification);
    rmsLabel.setJustificationType(juce::Justification::centred);
    rmsLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(18.0f)));
    addAndMakeVisible(rmsLabel);

    statusLabel.setText("WAITING FOR AUDIO SIGNAL", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(14.0f).withStyle("Bold")));
    addAndMakeVisible(statusLabel);

    testSignalButton.setButtonText("TEST SIGNAL: OFF");
    testSignalButton.onClick = [this]
    {
        const auto enabled = !audioProcessor.isTestSignalEnabled();
        audioProcessor.setTestSignalEnabled(enabled);
        testSignalButton.setButtonText(enabled ? "TEST SIGNAL: ON" : "TEST SIGNAL: OFF");
    };
    addAndMakeVisible(testSignalButton);

    addAndMakeVisible(analogMeter);

    setSize(800, 620);
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

    auto meterFrame = analogMeter.getBounds().toFloat().expanded(4.0f);
    g.setColour(juce::Colour(0xff353535));
    g.drawRoundedRectangle(meterFrame, 18.0f, 2.0f);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);

    titleLabel.setBounds(area.removeFromTop(48));

    auto readoutArea = area.removeFromTop(34);
    peakLabel.setBounds(readoutArea.removeFromLeft(readoutArea.getWidth() / 2));
    rmsLabel.setBounds(readoutArea);

    auto bottom = area.removeFromBottom(42);
    testSignalButton.setBounds(bottom.removeFromLeft(180).reduced(4));
    statusLabel.setBounds(bottom.reduced(4));

    analogMeter.setBounds(area.reduced(8, 4));
}

void MaFuckinHolyVUMeterAudioProcessorEditor::timerCallback()
{
    const auto peakDb = audioProcessor.getPeakDb();
    const auto rmsDb = audioProcessor.getRmsDb();

    peakLabel.setText(
        "PEAK: " + juce::String(peakDb, 1) + " dBFS",
        juce::dontSendNotification);

    rmsLabel.setText(
        "RMS: " + juce::String(rmsDb, 1) + " dBFS",
        juce::dontSendNotification);

    // Temporary visual mapping: RMS dBFS drives the analog needle.
    // Proper VU calibration and ballistics come in the meter-engine phase.
    analogMeter.setLevelDb(rmsDb);

    const auto testMode = audioProcessor.isTestSignalEnabled();
    const auto signalDetected = peakDb > -60.0f;

    if (testMode)
    {
        statusLabel.setText(
            "TEST SIGNAL ACTIVE • EXPECT ~ -12 dB PEAK / -15 dB RMS",
            juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText(
            signalDetected ? "AUDIO SIGNAL DETECTED" : "WAITING FOR AUDIO SIGNAL",
            juce::dontSendNotification);
    }
}

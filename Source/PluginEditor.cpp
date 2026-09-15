#include "PluginEditor.h"

namespace
{
void configureButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff171b1d));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb72b1c));
    button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd5d8d5));
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}
}

MaFuckinHolyVUMeterAudioProcessorEditor::MaFuckinHolyVUMeterAudioProcessorEditor(
    MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(25.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0dc));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("STEREO STUDIO METER", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f)));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    addAndMakeVisible(subtitleLabel);

    for (auto* readout : { &leftReadout, &rightReadout })
    {
        readout->setFont(juce::Font(juce::FontOptions{}.withHeight(13.0f)));
        readout->setColour(juce::Label::textColourId, juce::Colour(0xffd8dbd8));
        addAndMakeVisible(*readout);
    }

    statusLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f).withStyle("Bold")));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    statusLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(statusLabel);

    analogButton.setButtonText("ANALOG");
    digitalButton.setButtonText("DIGITAL");
    testSignalButton.setButtonText("TEST SIGNAL");
    for (auto* button : { &analogButton, &digitalButton, &testSignalButton })
    {
        configureButton(*button);
        addAndMakeVisible(*button);
    }

    analogButton.onClick = [this] { setDisplayMode(DisplayMode::analog); };
    digitalButton.onClick = [this] { setDisplayMode(DisplayMode::digital); };
    testSignalButton.onClick = [this]
    {
        audioProcessor.setTestSignalEnabled(!audioProcessor.isTestSignalEnabled());
    };

    addAndMakeVisible(leftAnalogMeter);
    addAndMakeVisible(rightAnalogMeter);
    addChildComponent(digitalMeter);

    setDisplayMode(DisplayMode::analog);
    setSize(920, 560);
    startTimerHz(30);
}

MaFuckinHolyVUMeterAudioProcessorEditor::~MaFuckinHolyVUMeterAudioProcessorEditor()
{
    stopTimer();
}

void MaFuckinHolyVUMeterAudioProcessorEditor::setDisplayMode(DisplayMode mode)
{
    displayMode = mode;
    const auto analog = mode == DisplayMode::analog;
    leftAnalogMeter.setVisible(analog);
    rightAnalogMeter.setVisible(analog);
    digitalMeter.setVisible(!analog);
    analogButton.setToggleState(analog, juce::dontSendNotification);
    digitalButton.setToggleState(!analog, juce::dontSendNotification);
    resized();
}

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111415));

    auto panel = getLocalBounds().toFloat().reduced(10.0f);
    g.setColour(juce::Colour(0xff202426));
    g.fillRoundedRectangle(panel, 10.0f);
    g.setColour(juce::Colour(0xff54595a));
    g.drawRoundedRectangle(panel, 10.0f, 1.0f);

    auto meterWindow = getLocalBounds().toFloat().reduced(28.0f, 102.0f);
    g.setColour(juce::Colour(0xff07090a));
    g.fillRoundedRectangle(meterWindow, 8.0f);
    g.setColour(juce::Colour(0xff3d4446));
    g.drawRoundedRectangle(meterWindow, 8.0f, 1.0f);

    g.setColour(juce::Colour(0xff5a5e5c));
    g.drawLine(28.0f, getHeight() - 52.0f, static_cast<float>(getWidth() - 28), getHeight() - 52.0f, 1.0f);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    auto header = area.removeFromTop(56);
    auto titleArea = header.removeFromLeft(360);
    titleLabel.setBounds(titleArea.removeFromTop(30));
    subtitleLabel.setBounds(titleArea);

    auto modes = header.removeFromRight(220);
    analogButton.setBounds(modes.removeFromLeft(100).reduced(3));
    digitalButton.setBounds(modes.reduced(3));

    auto footer = area.removeFromBottom(40);
    testSignalButton.setBounds(footer.removeFromLeft(130).reduced(2));
    leftReadout.setBounds(footer.removeFromLeft(210).reduced(4));
    rightReadout.setBounds(footer.removeFromLeft(210).reduced(4));
    statusLabel.setBounds(footer.reduced(4));

    auto meterArea = area.reduced(6, 6);
    if (displayMode == DisplayMode::analog)
    {
        auto left = meterArea.removeFromLeft(meterArea.getWidth() / 2);
        leftAnalogMeter.setBounds(left.reduced(5));
        rightAnalogMeter.setBounds(meterArea.reduced(5));
    }
    else
    {
        digitalMeter.setBounds(meterArea.reduced(5));
    }
}

void MaFuckinHolyVUMeterAudioProcessorEditor::timerCallback()
{
    const auto leftPeak = audioProcessor.getPeakDb(0);
    const auto rightPeak = audioProcessor.getPeakDb(1);
    const auto leftRms = audioProcessor.getRmsDb(0);
    const auto rightRms = audioProcessor.getRmsDb(1);

    leftAnalogMeter.setLevelDb(audioProcessor.getVuDb(0));
    leftAnalogMeter.setPeakDb(leftPeak);
    rightAnalogMeter.setLevelDb(audioProcessor.getVuDb(1));
    rightAnalogMeter.setPeakDb(rightPeak);
    digitalMeter.setLevels(leftPeak, leftRms, rightPeak, rightRms);

    leftReadout.setText("L  RMS " + juce::String(leftRms, 1) + " / PEAK "
                       + juce::String(leftPeak, 1) + " dBFS", juce::dontSendNotification);
    rightReadout.setText("R  RMS " + juce::String(rightRms, 1) + " / PEAK "
                        + juce::String(rightPeak, 1) + " dBFS", juce::dontSendNotification);

    const auto testMode = audioProcessor.isTestSignalEnabled();
    testSignalButton.setToggleState(testMode, juce::dontSendNotification);
    testSignalButton.setButtonText(testMode ? "TEST: ON" : "TEST SIGNAL");
    statusLabel.setText(testMode ? "440 Hz • -12 dBFS PEAK • ~ -15 dBFS RMS"
                                 : (juce::jmax(leftPeak, rightPeak) > -60.0f
                                     ? "STEREO SIGNAL DETECTED" : "WAITING FOR AUDIO SIGNAL"),
                        juce::dontSendNotification);
}

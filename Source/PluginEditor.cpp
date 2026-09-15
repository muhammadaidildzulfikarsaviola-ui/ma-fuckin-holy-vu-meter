#include "PluginEditor.h"

namespace
{
void configureButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff171b1d));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb72b1c));
    button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd5d8d5));
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void configureComboBox(juce::ComboBox& comboBox)
{
    comboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff111415));
    comboBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4b5153));
    comboBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffd9dcda));
    comboBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00bfe8));
}
}

MaFuckinHolyVUMeterAudioProcessorEditor::MaFuckinHolyVUMeterAudioProcessorEditor(
    MaFuckinHolyVUMeterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      standaloneMode(p.wrapperType == juce::AudioProcessor::wrapperType_Standalone),
      loopbackCapture([this](const float* samples, int frames, int channels, double sampleRate)
      { audioProcessor.processLoopbackSamples(samples, frames, channels, sampleRate); })
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(19.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0dc));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("TRUE PEAK  +  STEREO VU", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(9.0f)));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9da19f));
    addAndMakeVisible(subtitleLabel);

    statusLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(8.5f).withStyle("Bold")));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff858b88));
    statusLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(statusLabel);

    routingLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(8.5f)));
    routingLabel.setColour(juce::Label::textColourId, juce::Colour(0xff858b88));
    routingLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(routingLabel);

    configureButton(refreshDevicesButton);
    configureButton(connectLoopbackButton);
    configureButton(resetMetersButton);
    refreshDevicesButton.setButtonText("REFRESH");
    connectLoopbackButton.setButtonText("CONNECT");
    resetMetersButton.setButtonText("REFRESH");
    resetMetersButton.setTooltip("Refresh the meter peak memories");
    addAndMakeVisible(refreshDevicesButton);
    addAndMakeVisible(connectLoopbackButton);
    addAndMakeVisible(resetMetersButton);

    configureComboBox(sourceModeBox);
    configureComboBox(loopbackDeviceBox);
    sourceModeBox.addItem("NORMAL INPUT", 1);
    sourceModeBox.addItem("WASAPI LOOPBACK", 2);
    sourceModeBox.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(sourceModeBox);
    addAndMakeVisible(loopbackDeviceBox);

    sourceModeBox.onChange = [this] { updateMeterSource(); };
    loopbackDeviceBox.onChange = [this]
    {
        if (sourceModeBox.getSelectedId() == 2)
            updateMeterSource();
    };
    refreshDevicesButton.onClick = [this] { refreshLoopbackDevices(); };
    connectLoopbackButton.onClick = [this] { updateMeterSource(); };
    resetMetersButton.onClick = [this] { audioProcessor.resetMeterPeaks(); };

    addAndMakeVisible(peakLeftMeter);
    addAndMakeVisible(peakRightMeter);
    addAndMakeVisible(vuLeftMeter);
    addAndMakeVisible(vuRightMeter);

    if (standaloneMode)
        refreshLoopbackDevices();

    for (auto* component : { static_cast<juce::Component*>(&routingLabel),
                             static_cast<juce::Component*>(&sourceModeBox),
                             static_cast<juce::Component*>(&loopbackDeviceBox),
                             static_cast<juce::Component*>(&refreshDevicesButton),
                             static_cast<juce::Component*>(&connectLoopbackButton) })
        component->setVisible(standaloneMode);

    // Compact footprint. Standalone gets one extra routing row; VST3 stays small.
    setResizable(false, false);
    setSize(900, standaloneMode ? 610 : 520);
    startTimerHz(30);
}

MaFuckinHolyVUMeterAudioProcessorEditor::~MaFuckinHolyVUMeterAudioProcessorEditor()
{
    loopbackCapture.stopCapture();
    audioProcessor.setMeterSource(MaFuckinHolyVUMeterAudioProcessor::MeterSource::normalInput);
    stopTimer();
}

void MaFuckinHolyVUMeterAudioProcessorEditor::refreshLoopbackDevices()
{
    loopbackCapture.stopCapture();
    loopbackDevices = WasapiLoopbackCapture::enumerateRenderDevices();
    loopbackDeviceBox.clear(juce::dontSendNotification);

    for (size_t index = 0; index < loopbackDevices.size(); ++index)
        loopbackDeviceBox.addItem(loopbackDevices[index].name, static_cast<int>(index) + 1);

    if (loopbackDevices.empty())
    {
        loopbackDeviceBox.setTextWhenNothingSelected("NO WINDOWS RENDER ENDPOINTS");
        loopbackDeviceBox.setEnabled(false);
    }
    else
    {
        loopbackDeviceBox.setEnabled(true);
        loopbackDeviceBox.setSelectedId(1, juce::dontSendNotification);
    }
}

void MaFuckinHolyVUMeterAudioProcessorEditor::updateMeterSource()
{
    if (!standaloneMode || sourceModeBox.getSelectedId() != 2)
    {
        loopbackCapture.stopCapture();
        audioProcessor.setMeterSource(MaFuckinHolyVUMeterAudioProcessor::MeterSource::normalInput);
        return;
    }

    const auto deviceIndex = loopbackDeviceBox.getSelectedId() - 1;
    if (!juce::isPositiveAndBelow(deviceIndex, static_cast<int>(loopbackDevices.size())))
    {
        routingLabel.setText("LOOPBACK: choose a render endpoint", juce::dontSendNotification);
        return;
    }

    audioProcessor.setMeterSource(MaFuckinHolyVUMeterAudioProcessor::MeterSource::wasapiLoopback);
    loopbackCapture.startCapture(loopbackDevices[static_cast<size_t>(deviceIndex)]);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101314));

    auto panel = getLocalBounds().toFloat().reduced(7.0f);
    g.setColour(juce::Colour(0xff202426));
    g.fillRoundedRectangle(panel, 8.0f);
    g.setColour(juce::Colour(0xff515658));
    g.drawRoundedRectangle(panel, 8.0f, 1.0f);

    // Single compact meter bay instead of a large empty black canvas.
    auto meterWindow = panel.reduced(12.0f);
    meterWindow = meterWindow.withTrimmedTop(standaloneMode ? 83.0f : 48.0f)
                             .withTrimmedBottom(27.0f);
    g.setColour(juce::Colour(0xff090b0c));
    g.fillRoundedRectangle(meterWindow, 6.0f);
    g.setColour(juce::Colour(0xff3e4344));
    g.drawRoundedRectangle(meterWindow, 6.0f, 1.0f);

    g.setColour(juce::Colour(0xff3e4344));
    g.drawLine(meterWindow.getX() + 6.0f, meterWindow.getCentreY(),
               meterWindow.getRight() - 6.0f, meterWindow.getCentreY(), 1.0f);

    g.setColour(juce::Colour(0xff515658));
    g.drawLine(panel.getX() + 12.0f, panel.getBottom() - 21.0f,
               panel.getRight() - 12.0f, panel.getBottom() - 21.0f, 1.0f);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    auto header = area.removeFromTop(37);
    auto titleArea = header.removeFromLeft(360);
    titleLabel.setBounds(titleArea.removeFromTop(23));
    subtitleLabel.setBounds(titleArea);
    resetMetersButton.setBounds(header.removeFromRight(76).reduced(1, 3));

    if (standaloneMode)
    {
        auto routing = area.removeFromTop(35);
        sourceModeBox.setBounds(routing.removeFromLeft(132).reduced(2));
        loopbackDeviceBox.setBounds(routing.removeFromLeft(285).reduced(2));
        refreshDevicesButton.setBounds(routing.removeFromLeft(74).reduced(2));
        connectLoopbackButton.setBounds(routing.removeFromLeft(78).reduced(2));
        routingLabel.setBounds(routing.reduced(3));
    }

    auto footer = area.removeFromBottom(22);
    statusLabel.setBounds(footer.reduced(2));

    auto meterArea = area.reduced(3, 3);
    auto topRow = meterArea.removeFromTop(meterArea.getHeight() / 2);
    auto bottomRow = meterArea;

    auto peakLeftArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto peakRightArea = topRow;
    auto vuLeftArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto vuRightArea = bottomRow;

    peakLeftMeter.setBounds(peakLeftArea.reduced(3));
    peakRightMeter.setBounds(peakRightArea.reduced(3));
    vuLeftMeter.setBounds(vuLeftArea.reduced(3));
    vuRightMeter.setBounds(vuRightArea.reduced(3));
}

void MaFuckinHolyVUMeterAudioProcessorEditor::timerCallback()
{
    // -18 dBFS RMS sine is aligned to 0 VU by the VU engine.
    constexpr float vuCalibrationDb = 21.01f;

    const auto leftTruePeak = audioProcessor.getTruePeakCurrentDb(0);
    const auto rightTruePeak = audioProcessor.getTruePeakCurrentDb(1);
    const auto leftTruePeakMax = audioProcessor.getTruePeakMaxDb(0);
    const auto rightTruePeakMax = audioProcessor.getTruePeakMaxDb(1);
    const auto leftVu = audioProcessor.getVuDb(0) + vuCalibrationDb;
    const auto rightVu = audioProcessor.getVuDb(1) + vuCalibrationDb;
    const auto leftHold = audioProcessor.getVuPeakHoldDb(0) + vuCalibrationDb;
    const auto rightHold = audioProcessor.getVuPeakHoldDb(1) + vuCalibrationDb;

    peakLeftMeter.setLevelsDb(leftTruePeak, leftTruePeakMax);
    peakRightMeter.setLevelsDb(rightTruePeak, rightTruePeakMax);
    vuLeftMeter.setLevelsDb(leftVu, leftHold);
    vuRightMeter.setLevelsDb(rightVu, rightHold);

    if (standaloneMode)
    {
        if (sourceModeBox.getSelectedId() == 2)
            routingLabel.setText("LOOPBACK • " + loopbackCapture.getDeviceName()
                                 + " • " + loopbackCapture.getStatusText(), juce::dontSendNotification);
        else
            routingLabel.setText("INPUT • JUCE STANDALONE AUDIO", juce::dontSendNotification);
    }

    if (standaloneMode && sourceModeBox.getSelectedId() == 2)
        statusLabel.setText(loopbackCapture.isCapturing() ? "WASAPI LOOPBACK" : "LOOPBACK OFF",
                            juce::dontSendNotification);
    else
        statusLabel.setText((leftVu > -20.0f || rightVu > -20.0f)
                                ? "STEREO SIGNAL"
                                : "WAITING FOR SIGNAL",
                            juce::dontSendNotification);
}

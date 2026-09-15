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
      {
          audioProcessor.processLoopbackSamples(samples, frames, channels, sampleRate);
      })
{
    titleLabel.setText("MA FUCKIN HOLY VU METER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(25.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0dc));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("TRUE PEAK + STEREO VU", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f)));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    addAndMakeVisible(subtitleLabel);

    statusLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f).withStyle("Bold")));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    statusLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(statusLabel);

    routingLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f)));
    routingLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    addAndMakeVisible(routingLabel);

    configureMeterHeader(peakLeftHeader, "TRUE PEAK  L   |   CURRENT / MAX");
    configureMeterHeader(peakRightHeader, "TRUE PEAK  R   |   CURRENT / MAX");
    configureMeterHeader(vuLeftHeader, "VU  L   |   VU / PEAK HOLD");
    configureMeterHeader(vuRightHeader, "VU  R   |   VU / PEAK HOLD");
    addAndMakeVisible(peakLeftHeader);
    addAndMakeVisible(peakRightHeader);
    addAndMakeVisible(vuLeftHeader);
    addAndMakeVisible(vuRightHeader);

    refreshDevicesButton.setButtonText("REFRESH");
    connectLoopbackButton.setButtonText("CONNECT");
    configureButton(refreshDevicesButton);
    configureButton(connectLoopbackButton);
    addAndMakeVisible(refreshDevicesButton);
    addAndMakeVisible(connectLoopbackButton);

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

    setResizable(false, false);
    setSize(980, standaloneMode ? 850 : 760);
    startTimerHz(30);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::configureMeterHeader(juce::Label& label,
                                                                    const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions{}.withHeight(10.0f).withStyle("Bold")));
    label.setColour(juce::Label::textColourId, juce::Colour(0xff9fa5a2));
    label.setJustificationType(juce::Justification::centredLeft);
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
        routingLabel.setText("MODE: LOOPBACK • ERROR: choose a render endpoint", juce::dontSendNotification);
        return;
    }

    audioProcessor.setMeterSource(MaFuckinHolyVUMeterAudioProcessor::MeterSource::wasapiLoopback);
    loopbackCapture.startCapture(loopbackDevices[static_cast<size_t>(deviceIndex)]);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111415));
    auto panel = getLocalBounds().toFloat().reduced(10.0f);
    g.setColour(juce::Colour(0xff202426));
    g.fillRoundedRectangle(panel, 10.0f);
    g.setColour(juce::Colour(0xff54595a));
    g.drawRoundedRectangle(panel, 10.0f, 1.0f);

    auto meterWindow = getLocalBounds().toFloat().reduced(28.0f, 0.0f);
    meterWindow = meterWindow.withTrimmedTop(standaloneMode ? 112.0f : 68.0f)
                             .withTrimmedBottom(50.0f);
    g.setColour(juce::Colour(0xff07090a));
    g.fillRoundedRectangle(meterWindow, 8.0f);
    g.setColour(juce::Colour(0xff3d4446));
    g.drawRoundedRectangle(meterWindow, 8.0f, 1.0f);

    const auto separatorY = static_cast<float>(getHeight() / 2 + 18);
    g.setColour(juce::Colour(0xff3d4446));
    g.drawLine(40.0f, separatorY, static_cast<float>(getWidth() - 40), separatorY, 1.0f);
    g.setColour(juce::Colour(0xff5a5e5c));
    g.drawLine(28.0f, getHeight() - 44.0f,
               static_cast<float>(getWidth() - 28), getHeight() - 44.0f, 1.0f);
}

void MaFuckinHolyVUMeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    auto header = area.removeFromTop(56);
    auto titleArea = header.removeFromLeft(420);
    titleLabel.setBounds(titleArea.removeFromTop(30));
    subtitleLabel.setBounds(titleArea);

    if (standaloneMode)
    {
        auto routing = area.removeFromTop(46);
        sourceModeBox.setBounds(routing.removeFromLeft(150).reduced(2));
        loopbackDeviceBox.setBounds(routing.removeFromLeft(380).reduced(2));
        refreshDevicesButton.setBounds(routing.removeFromLeft(86).reduced(2));
        connectLoopbackButton.setBounds(routing.removeFromLeft(96).reduced(2));
        routingLabel.setBounds(routing.reduced(4));
    }

    auto footer = area.removeFromBottom(36);
    statusLabel.setBounds(footer.reduced(4));

    auto meterArea = area.reduced(6, 6);
    auto topRow = meterArea.removeFromTop(meterArea.getHeight() / 2);
    auto bottomRow = meterArea;

    auto peakLeftArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto peakRightArea = topRow;
    auto vuLeftArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto vuRightArea = bottomRow;

    peakLeftHeader.setBounds(peakLeftArea.removeFromTop(22).reduced(7, 2));
    peakLeftMeter.setBounds(peakLeftArea.reduced(5));
    peakRightHeader.setBounds(peakRightArea.removeFromTop(22).reduced(7, 2));
    peakRightMeter.setBounds(peakRightArea.reduced(5));
    vuLeftHeader.setBounds(vuLeftArea.removeFromTop(22).reduced(7, 2));
    vuLeftMeter.setBounds(vuLeftArea.reduced(5));
    vuRightHeader.setBounds(vuRightArea.removeFromTop(22).reduced(7, 2));
    vuRightMeter.setBounds(vuRightArea.reduced(5));
}

void MaFuckinHolyVUMeterAudioProcessorEditor::timerCallback()
{
    constexpr float vuCalibrationDb = 18.0f; // -18 dBFS = 0 VU

    // True peak is already in dBFS. Do not add VU calibration to it.
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
            routingLabel.setText("MODE: LOOPBACK • DEVICE: " + loopbackCapture.getDeviceName()
                                 + " • " + loopbackCapture.getStatusText(), juce::dontSendNotification);
        else
            routingLabel.setText("MODE: INPUT • JUCE STANDALONE AUDIO DEVICE", juce::dontSendNotification);
    }

    if (standaloneMode && sourceModeBox.getSelectedId() == 2)
        statusLabel.setText(loopbackCapture.isCapturing() ? "WASAPI LOOPBACK CAPTURING" : "WASAPI LOOPBACK NOT CONNECTED",
                            juce::dontSendNotification);
    else
        statusLabel.setText((leftVu > -20.0f || rightVu > -20.0f) ? "STEREO SIGNAL DETECTED" : "WAITING FOR AUDIO SIGNAL",
                            juce::dontSendNotification);
}

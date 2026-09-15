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

    routingLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(11.0f)));
    routingLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa8aaa7));
    addAndMakeVisible(routingLabel);

    analogButton.setButtonText("ANALOG");
    digitalButton.setButtonText("DIGITAL");
    testSignalButton.setButtonText("TEST SIGNAL");
    refreshDevicesButton.setButtonText("REFRESH");
    connectLoopbackButton.setButtonText("CONNECT");
    for (auto* button : { &analogButton, &digitalButton, &testSignalButton,
                          &refreshDevicesButton, &connectLoopbackButton })
    {
        configureButton(*button);
        addAndMakeVisible(*button);
    }

    configureComboBox(sourceModeBox);
    configureComboBox(loopbackDeviceBox);
    sourceModeBox.addItem("NORMAL INPUT", 1);
    sourceModeBox.addItem("WASAPI LOOPBACK", 2);
    sourceModeBox.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(sourceModeBox);
    addAndMakeVisible(loopbackDeviceBox);

    analogButton.onClick = [this] { setDisplayMode(DisplayMode::analog); };
    digitalButton.onClick = [this] { setDisplayMode(DisplayMode::digital); };
    testSignalButton.onClick = [this]
    {
        audioProcessor.setTestSignalEnabled(!audioProcessor.isTestSignalEnabled());
    };
    sourceModeBox.onChange = [this] { updateMeterSource(); };
    loopbackDeviceBox.onChange = [this]
    {
        if (sourceModeBox.getSelectedId() == 2)
            updateMeterSource();
    };
    refreshDevicesButton.onClick = [this] { refreshLoopbackDevices(); };
    connectLoopbackButton.onClick = [this] { updateMeterSource(); };

    addAndMakeVisible(leftAnalogMeter);
    addAndMakeVisible(rightAnalogMeter);
    addChildComponent(digitalMeter);

    if (standaloneMode)
        refreshLoopbackDevices();

    for (auto* component : { static_cast<juce::Component*>(&routingLabel),
                             static_cast<juce::Component*>(&sourceModeBox),
                             static_cast<juce::Component*>(&loopbackDeviceBox),
                             static_cast<juce::Component*>(&refreshDevicesButton),
                             static_cast<juce::Component*>(&connectLoopbackButton) })
        component->setVisible(standaloneMode);

    setDisplayMode(DisplayMode::analog);
    setSize(920, standaloneMode ? 630 : 560);
    startTimerHz(30);
}

MaFuckinHolyVUMeterAudioProcessorEditor::~MaFuckinHolyVUMeterAudioProcessorEditor()
{
    loopbackCapture.stopCapture();
    audioProcessor.setMeterSource(MaFuckinHolyVUMeterAudioProcessor::MeterSource::normalInput);
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
        routingLabel.setText("MODE: LOOPBACK • ERROR: choose a render endpoint",
                             juce::dontSendNotification);
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

    const auto meterTop = standaloneMode ? 154.0f : 102.0f;
    auto meterWindow = getLocalBounds().toFloat().reduced(28.0f, meterTop);
    g.setColour(juce::Colour(0xff07090a));
    g.fillRoundedRectangle(meterWindow, 8.0f);
    g.setColour(juce::Colour(0xff3d4446));
    g.drawRoundedRectangle(meterWindow, 8.0f, 1.0f);

    g.setColour(juce::Colour(0xff5a5e5c));
    g.drawLine(28.0f, getHeight() - 52.0f,
               static_cast<float>(getWidth() - 28), getHeight() - 52.0f, 1.0f);
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

    if (standaloneMode)
    {
        auto routing = area.removeFromTop(46);
        sourceModeBox.setBounds(routing.removeFromLeft(150).reduced(2));
        loopbackDeviceBox.setBounds(routing.removeFromLeft(380).reduced(2));
        refreshDevicesButton.setBounds(routing.removeFromLeft(86).reduced(2));
        connectLoopbackButton.setBounds(routing.removeFromLeft(96).reduced(2));
        routingLabel.setBounds(routing.reduced(4));
    }

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

    if (standaloneMode)
    {
        if (sourceModeBox.getSelectedId() == 2)
            routingLabel.setText("MODE: LOOPBACK • DEVICE: " + loopbackCapture.getDeviceName()
                                 + " • " + loopbackCapture.getStatusText(),
                                 juce::dontSendNotification);
        else
            routingLabel.setText("MODE: INPUT • JUCE STANDALONE DEVICE • configure driver/rate/buffer in Audio/MIDI Settings",
                                 juce::dontSendNotification);
    }

    if (testMode)
        statusLabel.setText("TEST SIGNAL • 440 Hz • -12 dBFS PEAK • ~ -15 dBFS RMS",
                            juce::dontSendNotification);
    else if (standaloneMode && sourceModeBox.getSelectedId() == 2)
        statusLabel.setText(loopbackCapture.isCapturing() ? "WASAPI LOOPBACK CAPTURING"
                                                           : "WASAPI LOOPBACK NOT CONNECTED",
                            juce::dontSendNotification);
    else
        statusLabel.setText(juce::jmax(leftPeak, rightPeak) > -60.0f
                                ? "STEREO SIGNAL DETECTED" : "WAITING FOR AUDIO SIGNAL",
                            juce::dontSendNotification);
}

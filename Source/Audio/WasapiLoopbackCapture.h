#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

class WasapiLoopbackCapture : private juce::Thread
{
public:
    struct DeviceInfo
    {
        juce::String id;
        juce::String name;
    };

    using SampleCallback = std::function<void(const float* interleavedSamples,
                                              int numFrames,
                                              int numChannels,
                                              double sampleRate)>;

    explicit WasapiLoopbackCapture(SampleCallback callback);
    ~WasapiLoopbackCapture() override;

    static std::vector<DeviceInfo> enumerateRenderDevices();

    bool startCapture(const DeviceInfo& device);
    void stopCapture();

    bool isCapturing() const noexcept;
    juce::String getStatusText() const;
    juce::String getDeviceName() const;

private:
    void run() override;
    void setStatus(juce::String text);

    SampleCallback sampleCallback;
    DeviceInfo selectedDevice;

    mutable juce::CriticalSection stateLock;
    juce::String statusText { "IDLE" };
    bool captureActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WasapiLoopbackCapture)
};

#include "WasapiLoopbackCapture.h"

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <mmdeviceapi.h>
 #include <audioclient.h>
 #include <functiondiscoverykeys_devpkey.h>
 #include <ksmedia.h>
#endif

namespace
{
#if JUCE_WINDOWS
template <typename T>
void releaseCom(T*& object)
{
    if (object != nullptr)
    {
        object->Release();
        object = nullptr;
    }
}

juce::String hresultText(HRESULT result)
{
    return juce::String::formatted("ERROR 0x%08X", static_cast<unsigned int>(result));
}

bool isFloatFormat(const WAVEFORMATEX& format)
{
    if (format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        return format.wBitsPerSample == 32;

    if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE
        && format.cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
    {
        const auto& extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE&>(format);
        return IsEqualGUID(extensible.SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            && format.wBitsPerSample == 32;
    }

    return false;
}

bool isPcmFormat(const WAVEFORMATEX& format)
{
    if (format.wFormatTag == WAVE_FORMAT_PCM)
        return true;

    if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE
        && format.cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
    {
        const auto& extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE&>(format);
        return IsEqualGUID(extensible.SubFormat, KSDATAFORMAT_SUBTYPE_PCM);
    }

    return false;
}

float pcmSampleToFloat(const BYTE* source, int bytesPerSample)
{
    switch (bytesPerSample)
    {
        case 2:
            return static_cast<float>(*reinterpret_cast<const int16_t*>(source)) / 32768.0f;

        case 3:
        {
            auto value = static_cast<int32_t>(source[0])
                       | (static_cast<int32_t>(source[1]) << 8)
                       | (static_cast<int32_t>(source[2]) << 16);
            if ((value & 0x00800000) != 0)
                value |= ~0x00ffffff;
            return static_cast<float>(value) / 8388608.0f;
        }

        case 4:
            return static_cast<float>(*reinterpret_cast<const int32_t*>(source)) / 2147483648.0f;

        default:
            return 0.0f;
    }
}
#endif
}

WasapiLoopbackCapture::WasapiLoopbackCapture(SampleCallback callback)
    : juce::Thread("WASAPI Loopback Capture"),
      sampleCallback(std::move(callback))
{
}

WasapiLoopbackCapture::~WasapiLoopbackCapture()
{
    stopCapture();
}

std::vector<WasapiLoopbackCapture::DeviceInfo>
WasapiLoopbackCapture::enumerateRenderDevices()
{
    std::vector<DeviceInfo> devices;

#if JUCE_WINDOWS
    HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitialise = SUCCEEDED(comResult);

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* collection = nullptr;

    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                   __uuidof(IMMDeviceEnumerator),
                                   reinterpret_cast<void**>(&enumerator)))
        && SUCCEEDED(enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection)))
    {
        UINT count = 0;
        collection->GetCount(&count);

        for (UINT index = 0; index < count; ++index)
        {
            IMMDevice* device = nullptr;
            LPWSTR id = nullptr;
            IPropertyStore* properties = nullptr;
            PROPVARIANT name;
            PropVariantInit(&name);

            if (SUCCEEDED(collection->Item(index, &device))
                && SUCCEEDED(device->GetId(&id))
                && SUCCEEDED(device->OpenPropertyStore(STGM_READ, &properties))
                && SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &name))
                && name.vt == VT_LPWSTR)
            {
                devices.push_back({ juce::String(id), juce::String(name.pwszVal) });
            }

            PropVariantClear(&name);
            if (id != nullptr) CoTaskMemFree(id);
            releaseCom(properties);
            releaseCom(device);
        }
    }

    releaseCom(collection);
    releaseCom(enumerator);
    if (shouldUninitialise) CoUninitialize();
#endif

    return devices;
}

bool WasapiLoopbackCapture::startCapture(const DeviceInfo& device)
{
#if JUCE_WINDOWS
    if (device.id.isEmpty())
    {
        setStatus("ERROR: select a Windows render device");
        return false;
    }

    stopCapture();

    {
        const juce::ScopedLock lock(stateLock);
        selectedDevice = device;
        statusText = "CONNECTING...";
        captureActive = false;
    }

    startThread(juce::Thread::Priority::high);
    return true;
#else
    juce::ignoreUnused(device);
    setStatus("ERROR: WASAPI is available on Windows only");
    return false;
#endif
}

void WasapiLoopbackCapture::stopCapture()
{
    signalThreadShouldExit();
    stopThread(2000);

    const juce::ScopedLock lock(stateLock);
    captureActive = false;
    if (!statusText.startsWith("ERROR"))
        statusText = "IDLE";
}

bool WasapiLoopbackCapture::isCapturing() const noexcept
{
    const juce::ScopedLock lock(stateLock);
    return captureActive;
}

juce::String WasapiLoopbackCapture::getStatusText() const
{
    const juce::ScopedLock lock(stateLock);
    return statusText;
}

juce::String WasapiLoopbackCapture::getDeviceName() const
{
    const juce::ScopedLock lock(stateLock);
    return selectedDevice.name;
}

void WasapiLoopbackCapture::setStatus(juce::String text)
{
    const juce::ScopedLock lock(stateLock);
    statusText = std::move(text);
}

void WasapiLoopbackCapture::run()
{
#if JUCE_WINDOWS
    HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitialise = SUCCEEDED(comResult);

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioCaptureClient* captureClient = nullptr;
    WAVEFORMATEX* mixFormat = nullptr;

    const auto fail = [this](HRESULT result)
    {
        setStatus("WASAPI " + hresultText(result));
    };

    HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator),
                                      reinterpret_cast<void**>(&enumerator));

    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    {
        juce::String deviceId;
        {
            const juce::ScopedLock lock(stateLock);
            deviceId = selectedDevice.id;
        }
        result = enumerator->GetDevice(deviceId.toWideCharPointer(), &device);
    }

    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    result = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void**>(&audioClient));
    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    result = audioClient->GetMixFormat(&mixFormat);
    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    result = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                     0, 0, mixFormat, nullptr);
    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    result = audioClient->GetService(__uuidof(IAudioCaptureClient),
                                     reinterpret_cast<void**>(&captureClient));
    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    result = audioClient->Start();
    if (FAILED(result))
    {
        fail(result);
        goto cleanup;
    }

    {
        const juce::ScopedLock lock(stateLock);
        captureActive = true;
        statusText = "CONNECTED • " + juce::String(mixFormat->nSamplesPerSec)
                   + " Hz • " + juce::String(mixFormat->nChannels) + " ch";
    }

    {
        std::vector<float> convertedSamples;
        const auto floatSamples = isFloatFormat(*mixFormat);
        const auto pcmSamples = isPcmFormat(*mixFormat);
        const auto bytesPerSample = static_cast<int>(mixFormat->wBitsPerSample / 8);

        if (!floatSamples && (!pcmSamples || (bytesPerSample != 2 && bytesPerSample != 3 && bytesPerSample != 4)))
        {
            setStatus("ERROR: unsupported WASAPI mix format");
            goto cleanup;
        }

        while (!threadShouldExit())
        {
            UINT32 packetFrames = 0;
            result = captureClient->GetNextPacketSize(&packetFrames);
            if (FAILED(result))
            {
                fail(result);
                break;
            }

            if (packetFrames == 0)
            {
                wait(5);
                continue;
            }

            while (packetFrames > 0 && !threadShouldExit())
            {
                BYTE* data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                result = captureClient->GetBuffer(&data, &frames, &flags, nullptr, nullptr);
                if (FAILED(result))
                {
                    fail(result);
                    break;
                }

                if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0)
                {
                    sampleCallback(nullptr, static_cast<int>(frames),
                                   static_cast<int>(mixFormat->nChannels),
                                   static_cast<double>(mixFormat->nSamplesPerSec));
                }
                else if (floatSamples)
                {
                    sampleCallback(reinterpret_cast<const float*>(data), static_cast<int>(frames),
                                   static_cast<int>(mixFormat->nChannels),
                                   static_cast<double>(mixFormat->nSamplesPerSec));
                }
                else
                {
                    convertedSamples.resize(static_cast<size_t>(frames) * mixFormat->nChannels);
                    for (UINT32 frame = 0; frame < frames; ++frame)
                        for (WORD channel = 0; channel < mixFormat->nChannels; ++channel)
                            convertedSamples[static_cast<size_t>(frame) * mixFormat->nChannels + channel]
                                = pcmSampleToFloat(data + (static_cast<size_t>(frame) * mixFormat->nBlockAlign)
                                                    + channel * bytesPerSample, bytesPerSample);

                    sampleCallback(convertedSamples.data(), static_cast<int>(frames),
                                   static_cast<int>(mixFormat->nChannels),
                                   static_cast<double>(mixFormat->nSamplesPerSec));
                }

                captureClient->ReleaseBuffer(frames);
                result = captureClient->GetNextPacketSize(&packetFrames);
                if (FAILED(result))
                {
                    fail(result);
                    break;
                }
            }
        }
    }

cleanup:
    if (audioClient != nullptr)
        audioClient->Stop();

    releaseCom(captureClient);
    if (mixFormat != nullptr) CoTaskMemFree(mixFormat);
    releaseCom(audioClient);
    releaseCom(device);
    releaseCom(enumerator);

    {
        const juce::ScopedLock lock(stateLock);
        captureActive = false;
        if (!statusText.startsWith("WASAPI ERROR") && !statusText.startsWith("ERROR"))
            statusText = "IDLE";
    }

    if (shouldUninitialise) CoUninitialize();
#else
    setStatus("ERROR: WASAPI is available on Windows only");
#endif
}

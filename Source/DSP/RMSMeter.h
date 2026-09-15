#pragma once

#include <JuceHeader.h>

class RMSMeter
{
public:
    void prepare(double sampleRate);
    void reset();
    void process(const juce::AudioBuffer<float>& buffer);
    void processTestSignal(int numSamples, double sampleRate, double& phase);

    float getRmsDb() const noexcept;
    float getRmsLinear() const noexcept;

private:
    std::atomic<float> rmsLinear { 0.0f };
};

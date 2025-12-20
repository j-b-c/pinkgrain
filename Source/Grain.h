#pragma once

#include <JuceHeader.h>

class Grain
{
public:
    Grain();

    void start(const juce::AudioBuffer<float>& sourceBuffer,
               double sourceSampleRate,
               int startSampleInSource,
               int grainLengthSamples,
               float pitchRatio,
               float pan,
               float attackSamples,
               float releaseSamples,
               bool reverse,
               float velocity);

    void process(juce::AudioBuffer<float>& outputBuffer,
                 int startSample,
                 int numSamples,
                 double outputSampleRate);

    bool isActive() const { return active; }
    bool isDone() const { return done; }

    float getCurrentPosition() const;
    float getEnvelopeLevel() const { return currentEnvelopeLevel; }
    int getStartSampleInSource() const { return sourceSampleStart; }
    int getGrainLength() const { return grainLength; }
    float getProgress() const { return grainLength > 0 ? static_cast<float>(samplesProcessed) / static_cast<float>(grainLength) : 0.0f; }
    int getSourceLength() const { return source ? source->getNumSamples() : 0; }

private:
    float getEnvelope(int sampleIndex) const;
    float interpolateSample(const juce::AudioBuffer<float>& buffer, int channel, double position) const;

    const juce::AudioBuffer<float>* source = nullptr;
    double sourceSampleRate = 44100.0;

    int sourceSampleStart = 0;
    int grainLength = 0;
    float pitchRatio = 1.0f;
    float panLeft = 1.0f;
    float panRight = 1.0f;
    float attackSamples = 0.0f;
    float releaseSamples = 0.0f;
    bool reverse = false;
    float velocity = 1.0f;

    double currentPosition = 0.0;
    int samplesProcessed = 0;
    float currentEnvelopeLevel = 0.0f;

    bool active = false;
    bool done = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Grain)
};

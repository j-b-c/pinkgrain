#include "Grain.h"

Grain::Grain()
{
}

void Grain::start(const juce::AudioBuffer<float>& sourceBuffer,
                  double srcSampleRate,
                  int startSampleInSource,
                  int grainLengthSamples,
                  float pitch,
                  float pan,
                  float attack,
                  float release,
                  bool rev,
                  float vel,
                  int midiNoteNumber)
{
    source = &sourceBuffer;
    sourceSampleRate = srcSampleRate;
    sourceSampleStart = startSampleInSource;
    grainLength = grainLengthSamples;
    pitchRatio = pitch;
    attackSamples = attack;
    releaseSamples = release;
    reverse = rev;
    velocity = vel;
    midiNote = midiNoteNumber;

    // Equal power panning
    float panAngle = pan * juce::MathConstants<float>::halfPi * 0.5f;
    panLeft = std::cos(panAngle + juce::MathConstants<float>::halfPi * 0.25f);
    panRight = std::sin(panAngle + juce::MathConstants<float>::halfPi * 0.25f);

    currentPosition = reverse ? static_cast<double>(grainLength - 1) : 0.0;
    samplesProcessed = 0;
    currentEnvelopeLevel = 0.0f;

    active = true;
    done = false;

    // Reset release state
    releasing = false;
    releaseSampleStart = 0;
    releaseStartLevel = 0.0f;
}

void Grain::process(juce::AudioBuffer<float>& outputBuffer,
                    int startSample,
                    int numSamples,
                    double outputSampleRate)
{
    if (!active || done || source == nullptr)
        return;

    const int numSourceChannels = source->getNumChannels();
    const int sourceLength = source->getNumSamples();
    const double sampleRateRatio = sourceSampleRate / outputSampleRate;
    const double positionIncrement = pitchRatio * sampleRateRatio * (reverse ? -1.0 : 1.0);

    for (int i = 0; i < numSamples; ++i)
    {
        // Check if we've reached the end of the grain or finished releasing
        if (samplesProcessed >= grainLength)
        {
            active = false;
            done = true;
            return;
        }

        // Calculate envelope
        currentEnvelopeLevel = getEnvelope();

        // If releasing and envelope has reached zero, we're done
        if (releasing && currentEnvelopeLevel <= 0.001f)
        {
            active = false;
            done = true;
            return;
        }

        // Get source position
        int sourcePos = sourceSampleStart + static_cast<int>(currentPosition);

        // Check bounds
        if (sourcePos < 0 || sourcePos >= sourceLength)
        {
            currentPosition += positionIncrement;
            ++samplesProcessed;
            continue;
        }

        // Get samples from source (with interpolation)
        float leftSample = 0.0f;
        float rightSample = 0.0f;

        double actualPos = sourceSampleStart + currentPosition;

        if (numSourceChannels == 1)
        {
            // Mono source - use same sample for both channels
            float sample = interpolateSample(*source, 0, actualPos);
            leftSample = sample;
            rightSample = sample;
        }
        else
        {
            // Stereo source
            leftSample = interpolateSample(*source, 0, actualPos);
            rightSample = interpolateSample(*source, 1, actualPos);
        }

        // Apply envelope, velocity, and panning
        leftSample *= currentEnvelopeLevel * velocity * panLeft;
        rightSample *= currentEnvelopeLevel * velocity * panRight;

        // Mix into output buffer
        int outputSample = startSample + i;
        if (outputSample < outputBuffer.getNumSamples())
        {
            outputBuffer.addSample(0, outputSample, leftSample);
            outputBuffer.addSample(1, outputSample, rightSample);
        }

        currentPosition += positionIncrement;
        ++samplesProcessed;
    }
}

void Grain::triggerRelease()
{
    if (!active || releasing)
        return;

    releasing = true;
    releaseSampleStart = samplesProcessed;
    releaseStartLevel = currentEnvelopeLevel;
}

float Grain::getEnvelope()
{
    if (grainLength <= 0)
        return 0.0f;

    // If we're in early release mode (triggered by note-off)
    if (releasing)
    {
        int samplesSinceRelease = samplesProcessed - releaseSampleStart;
        if (releaseSamples > 0.0f && samplesSinceRelease < static_cast<int>(releaseSamples))
        {
            float releaseProgress = static_cast<float>(samplesSinceRelease) / releaseSamples;
            float env = releaseStartLevel * (1.0f - releaseProgress);
            return 0.5f * (1.0f - std::cos(env * juce::MathConstants<float>::pi));
        }
        return 0.0f;
    }

    float env = 1.0f;

    // Attack phase
    if (attackSamples > 0.0f && samplesProcessed < static_cast<int>(attackSamples))
    {
        env = static_cast<float>(samplesProcessed) / attackSamples;
    }
    // Natural release phase (at end of grain)
    else if (releaseSamples > 0.0f)
    {
        int releaseStart = grainLength - static_cast<int>(releaseSamples);
        if (samplesProcessed >= releaseStart)
        {
            env = 1.0f - (static_cast<float>(samplesProcessed - releaseStart) / releaseSamples);
        }
    }

    // Apply a slight smoothing curve (raised cosine)
    return 0.5f * (1.0f - std::cos(env * juce::MathConstants<float>::pi));
}

float Grain::interpolateSample(const juce::AudioBuffer<float>& buffer, int channel, double position) const
{
    const int numSamples = buffer.getNumSamples();

    if (position < 0.0 || position >= numSamples - 1)
        return 0.0f;

    int index0 = static_cast<int>(position);
    int index1 = index0 + 1;
    float frac = static_cast<float>(position - index0);

    if (index1 >= numSamples)
        index1 = index0;

    const float* data = buffer.getReadPointer(channel);
    return data[index0] + frac * (data[index1] - data[index0]);
}

float Grain::getCurrentPosition() const
{
    if (source == nullptr || source->getNumSamples() == 0)
        return 0.0f;

    return static_cast<float>(sourceSampleStart + currentPosition) / static_cast<float>(source->getNumSamples());
}

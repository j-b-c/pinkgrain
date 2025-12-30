#pragma once

#include <JuceHeader.h>
#include "Grain.h"

struct GrainInfo
{
    float normalizedPosition;      // Position in the source file (0-1)
    float grainProgress;           // Progress through the grain (0-1)
    float envelopeLevel;           // Current envelope amplitude
    float grainStartPosition;      // Where this grain started in the source
    float grainEndPosition;        // Where this grain ends in the source
    int midiNote;                  // MIDI note number that triggered this grain
    bool active;
};

class GrainEngine
{
public:
    GrainEngine();
    ~GrainEngine();

    void prepare(double sampleRate, int samplesPerBlock);
    void setSourceBuffer(const juce::AudioBuffer<float>* buffer, double sourceSampleRate);

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void allNotesOff();

    void process(juce::AudioBuffer<float>& outputBuffer);

    // Parameters
    void setGrainSize(float sizeMs);
    void setDensity(float grainsPerSecond);
    void setPosition(float normalizedPosition);
    void setPitch(float semitones);
    void setPanSpread(float spread);
    void setAttack(float attackMs);
    void setDecay(float decayMs);
    void setSustain(float sustainLevel);
    void setRelease(float releaseMs);
    void setReverse(bool reverse);
    void setSpray(float spray);
    void setPitchRandom(float randomSemitones);
    void setVolume(float volume);
    void setMaxActiveGrains(int maxGrains);

    // For UI visualization
    std::vector<GrainInfo> getActiveGrainInfo() const;
    int getNumActiveGrains() const;

    // Root note for pitch calculation (middle C)
    static constexpr int ROOT_NOTE = 60;

private:
    void spawnGrain(int midiNote, float velocity);
    Grain* getInactiveGrain();

    static constexpr int MAX_GRAINS = 2048;  // Absolute maximum
    std::array<std::unique_ptr<Grain>, MAX_GRAINS> grains;
    int maxActiveGrains = 512;  // User-configurable active pool size

    const juce::AudioBuffer<float>* sourceBuffer = nullptr;
    double sourceSampleRate = 44100.0;
    double outputSampleRate = 44100.0;

    // Parameters
    float grainSizeMs = 100.0f;
    float density = 10.0f;
    float position = 0.0f;
    float pitchSemitones = 0.0f;
    float panSpread = 0.5f;
    float attackMs = 10.0f;
    float decayMs = 50.0f;
    float sustainLevel = 0.8f;
    float releaseMs = 50.0f;
    bool reverse = false;
    float spray = 0.0f;
    float pitchRandom = 0.0f;
    float volume = 1.0f;

    // Note tracking
    std::map<int, float> activeNotes;  // midiNote -> velocity

    // Grain spawning
    double samplesUntilNextGrain = 0.0;

    juce::Random random;

    juce::CriticalSection grainLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainEngine)
};

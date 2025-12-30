#include "GrainEngine.h"

GrainEngine::GrainEngine()
{
    for (auto& grain : grains)
    {
        grain = std::make_unique<Grain>();
    }
}

GrainEngine::~GrainEngine()
{
}

void GrainEngine::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    outputSampleRate = sampleRate;
    samplesUntilNextGrain = 0.0;
}

void GrainEngine::setSourceBuffer(const juce::AudioBuffer<float>* buffer, double srcSampleRate)
{
    juce::ScopedLock lock(grainLock);
    sourceBuffer = buffer;
    sourceSampleRate = srcSampleRate;
}

void GrainEngine::noteOn(int midiNote, float velocity)
{
    activeNotes[midiNote] = velocity;
}

void GrainEngine::noteOff(int midiNote)
{
    activeNotes.erase(midiNote);

    // Trigger release only on grains that belong to this specific note
    juce::ScopedLock lock(grainLock);
    for (auto& grain : grains)
    {
        if (grain->isActive() && grain->getMidiNote() == midiNote)
        {
            grain->triggerRelease();
        }
    }
}

void GrainEngine::allNotesOff()
{
    activeNotes.clear();

    // Trigger release on all active grains
    juce::ScopedLock lock(grainLock);
    for (auto& grain : grains)
    {
        if (grain->isActive())
        {
            grain->triggerRelease();
        }
    }
}

void GrainEngine::process(juce::AudioBuffer<float>& outputBuffer)
{
    juce::ScopedLock lock(grainLock);

    if (sourceBuffer == nullptr || sourceBuffer->getNumSamples() == 0)
        return;

    const int numSamples = outputBuffer.getNumSamples();

    // Spawn new grains if notes are active
    if (!activeNotes.empty() && density > 0.0f)
    {
        // Each active note contributes to the total density
        double effectiveDensity = density * static_cast<double>(activeNotes.size());
        double samplesPerGrain = outputSampleRate / effectiveDensity;

        for (int i = 0; i < numSamples; ++i)
        {
            samplesUntilNextGrain -= 1.0;

            if (samplesUntilNextGrain <= 0.0)
            {
                // Spawn a grain for a randomly selected active note
                // This distributes grains across all held notes
                int noteIndex = random.nextInt(static_cast<int>(activeNotes.size()));
                auto it = activeNotes.begin();
                std::advance(it, noteIndex);
                spawnGrain(it->first, it->second);
                samplesUntilNextGrain += samplesPerGrain;
            }
        }
    }

    // Process all active grains
    for (auto& grain : grains)
    {
        if (grain->isActive())
        {
            grain->process(outputBuffer, 0, numSamples, outputSampleRate);
        }
    }

    // Apply master volume
    outputBuffer.applyGain(volume);
}

void GrainEngine::spawnGrain(int midiNote, float velocity)
{
    Grain* grain = getInactiveGrain();
    if (grain == nullptr || sourceBuffer == nullptr)
        return;

    const int sourceLengthSamples = sourceBuffer->getNumSamples();

    // Calculate grain parameters
    int grainLengthSamples = static_cast<int>((grainSizeMs / 1000.0) * sourceSampleRate);
    grainLengthSamples = juce::jlimit(1, sourceLengthSamples, grainLengthSamples);

    // Position with spray (randomness)
    float actualPosition = position;
    if (spray > 0.0f)
    {
        float sprayAmount = (random.nextFloat() * 2.0f - 1.0f) * spray;
        actualPosition = juce::jlimit(0.0f, 1.0f, position + sprayAmount);
    }

    int startSample = static_cast<int>(actualPosition * static_cast<float>(sourceLengthSamples - grainLengthSamples));
    startSample = juce::jlimit(0, sourceLengthSamples - grainLengthSamples, startSample);

    // Calculate pitch from MIDI note relative to root note (middle C = 60)
    // Plus the pitch dial offset and randomness
    float notePitchSemitones = static_cast<float>(midiNote - ROOT_NOTE);
    float actualPitch = notePitchSemitones + pitchSemitones;
    if (pitchRandom > 0.0f)
    {
        float pitchRandomAmount = (random.nextFloat() * 2.0f - 1.0f) * pitchRandom;
        actualPitch += pitchRandomAmount;
    }
    float pitchRatio = std::pow(2.0f, actualPitch / 12.0f);

    // Pan with spread
    float pan = 0.5f;
    if (panSpread > 0.0f)
    {
        pan = 0.5f + (random.nextFloat() - 0.5f) * panSpread;
    }

    // ADSR envelope in samples
    float attackSamples = (attackMs / 1000.0f) * static_cast<float>(sourceSampleRate);
    float decaySamples = (decayMs / 1000.0f) * static_cast<float>(sourceSampleRate);
    float releaseSamples = (releaseMs / 1000.0f) * static_cast<float>(sourceSampleRate);

    // Ensure attack + decay + release don't exceed grain length
    float totalEnvSamples = attackSamples + decaySamples + releaseSamples;
    if (totalEnvSamples > grainLengthSamples)
    {
        float scale = static_cast<float>(grainLengthSamples) / totalEnvSamples;
        attackSamples *= scale;
        decaySamples *= scale;
        releaseSamples *= scale;
    }

    grain->start(*sourceBuffer, sourceSampleRate, startSample, grainLengthSamples,
                 pitchRatio, pan, attackSamples, decaySamples, sustainLevel, releaseSamples,
                 reverse, velocity, midiNote);
}

Grain* GrainEngine::getInactiveGrain()
{
    // First, try to find an inactive grain within the active pool
    for (int i = 0; i < maxActiveGrains; ++i)
    {
        if (!grains[i]->isActive())
        {
            return grains[i].get();
        }
    }

    // If all active grains are in use, steal the one that's furthest along (voice stealing)
    Grain* oldestGrain = nullptr;
    float maxProgress = 0.0f;

    for (int i = 0; i < maxActiveGrains; ++i)
    {
        float progress = grains[i]->getProgress();
        if (progress > maxProgress)
        {
            maxProgress = progress;
            oldestGrain = grains[i].get();
        }
    }

    return oldestGrain;  // Will reuse the grain closest to completion
}

void GrainEngine::setGrainSize(float sizeMs)
{
    grainSizeMs = juce::jlimit(10.0f, 30000.0f, sizeMs);
}

void GrainEngine::setDensity(float grainsPerSecond)
{
    density = juce::jlimit(1.0f, 100.0f, grainsPerSecond);
}

void GrainEngine::setPosition(float normalizedPosition)
{
    position = juce::jlimit(0.0f, 1.0f, normalizedPosition);
}

void GrainEngine::setPitch(float semitones)
{
    pitchSemitones = juce::jlimit(-24.0f, 24.0f, semitones);
}

void GrainEngine::setPanSpread(float spread)
{
    panSpread = juce::jlimit(0.0f, 1.0f, spread);
}

void GrainEngine::setAttack(float attack)
{
    attackMs = juce::jlimit(0.0f, 100.0f, attack);
}

void GrainEngine::setDecay(float decay)
{
    decayMs = juce::jlimit(0.0f, 500.0f, decay);
}

void GrainEngine::setSustain(float sustain)
{
    sustainLevel = juce::jlimit(0.0f, 1.0f, sustain);
}

void GrainEngine::setRelease(float release)
{
    releaseMs = juce::jlimit(0.0f, 5000.0f, release);
}

void GrainEngine::setReverse(bool rev)
{
    reverse = rev;
}

void GrainEngine::setSpray(float sprayAmount)
{
    spray = juce::jlimit(0.0f, 1.0f, sprayAmount);
}

void GrainEngine::setPitchRandom(float randomSemitones)
{
    pitchRandom = juce::jlimit(0.0f, 24.0f, randomSemitones);
}

void GrainEngine::setVolume(float vol)
{
    volume = juce::jlimit(0.0f, 1.0f, vol);
}

void GrainEngine::setMaxActiveGrains(int maxGrains)
{
    maxActiveGrains = juce::jlimit(64, MAX_GRAINS, maxGrains);
}

std::vector<GrainInfo> GrainEngine::getActiveGrainInfo() const
{
    std::vector<GrainInfo> info;
    info.reserve(MAX_GRAINS);

    for (const auto& grain : grains)
    {
        if (grain->isActive())
        {
            GrainInfo gi;
            gi.normalizedPosition = grain->getCurrentPosition();
            gi.grainProgress = grain->getProgress();
            gi.envelopeLevel = grain->getEnvelopeLevel();
            gi.midiNote = grain->getMidiNote();

            int sourceLength = grain->getSourceLength();
            if (sourceLength > 0)
            {
                gi.grainStartPosition = static_cast<float>(grain->getStartSampleInSource()) / static_cast<float>(sourceLength);
                gi.grainEndPosition = static_cast<float>(grain->getStartSampleInSource() + grain->getGrainLength()) / static_cast<float>(sourceLength);
            }
            else
            {
                gi.grainStartPosition = 0.0f;
                gi.grainEndPosition = 0.0f;
            }

            gi.active = true;
            info.push_back(gi);
        }
    }

    return info;
}

int GrainEngine::getNumActiveGrains() const
{
    int count = 0;
    for (const auto& grain : grains)
    {
        if (grain->isActive())
            ++count;
    }
    return count;
}

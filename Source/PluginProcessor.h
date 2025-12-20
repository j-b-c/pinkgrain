#pragma once

#include <JuceHeader.h>
#include "GrainEngine.h"
#include "AudioFileLoader.h"

class PinkGrainAudioProcessor : public juce::AudioProcessor
{
public:
    PinkGrainAudioProcessor();
    ~PinkGrainAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Access to components
    GrainEngine& getGrainEngine() { return grainEngine; }
    AudioFileLoader& getAudioFileLoader() { return audioFileLoader; }
    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }

    // Parameter IDs
    static const juce::String GRAIN_SIZE_ID;
    static const juce::String DENSITY_ID;
    static const juce::String POSITION_ID;
    static const juce::String PITCH_ID;
    static const juce::String PAN_SPREAD_ID;
    static const juce::String ATTACK_ID;
    static const juce::String RELEASE_ID;
    static const juce::String REVERSE_ID;
    static const juce::String SPRAY_ID;
    static const juce::String PITCH_RANDOM_ID;
    static const juce::String VOLUME_ID;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateGrainEngineParameters();

    GrainEngine grainEngine;
    AudioFileLoader audioFileLoader;

    juce::AudioProcessorValueTreeState apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinkGrainAudioProcessor)
};

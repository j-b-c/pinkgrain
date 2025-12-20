#pragma once

#include <JuceHeader.h>
#include "../GrainEngine.h"
#include "../AudioFileLoader.h"

class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay(AudioFileLoader& loader, GrainEngine& engine);
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

    void setPositionParameter(std::atomic<float>* positionParam);
    void setGrainSizeParameter(std::atomic<float>* grainSizeParam);
    void setSourceSampleRate(double sampleRate);
    void setSourceLengthSamples(int lengthSamples);

private:
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrainWindow(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrains(juce::Graphics& g, juce::Rectangle<int> bounds);

    AudioFileLoader& audioFileLoader;
    GrainEngine& grainEngine;

    std::atomic<float>* positionParameter = nullptr;
    std::atomic<float>* grainSizeParameter = nullptr;
    double sourceSampleRate = 44100.0;
    int sourceLengthSamples = 0;

    // Cached grain info for thread-safe rendering
    std::vector<GrainInfo> cachedGrainInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

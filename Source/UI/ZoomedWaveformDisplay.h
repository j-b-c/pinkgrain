#pragma once

#include <JuceHeader.h>
#include "../GrainEngine.h"
#include "../AudioFileLoader.h"

class ZoomedWaveformDisplay : public juce::Component
{
public:
    ZoomedWaveformDisplay(AudioFileLoader& loader, GrainEngine& engine);
    ~ZoomedWaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPositionParameter(std::atomic<float>* positionParam);
    void setGrainSizeParameter(std::atomic<float>* grainSizeParam);

private:
    void onVBlank();
    void drawZoomedWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrains(juce::Graphics& g, juce::Rectangle<int> bounds);

    AudioFileLoader& audioFileLoader;
    GrainEngine& grainEngine;

    std::atomic<float>* positionParameter = nullptr;
    std::atomic<float>* grainSizeParameter = nullptr;

    // Cached grain info for thread-safe rendering
    std::vector<GrainInfo> cachedGrainInfo;

    // VBlank sync for display refresh rate
    juce::VBlankAttachment vBlankAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZoomedWaveformDisplay)
};

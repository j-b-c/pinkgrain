#pragma once

#include <JuceHeader.h>
#include "../AudioFileLoader.h"

class GrainEngine;

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay(AudioFileLoader& loader, GrainEngine& engine);
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Mouse interaction
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setPositionParameter(std::atomic<float>* positionParam);
    void setGrainSizeParameter(std::atomic<float>* grainSizeParam);
    void setSourceSampleRate(double sampleRate);
    void setSourceLengthSamples(int lengthSamples);

    // Callback for position changes from mouse drag
    std::function<void(float)> onPositionChanged;
    std::function<void(float)> onSizeChanged;

private:
    void onVBlank();
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrainWindow(juce::Graphics& g, juce::Rectangle<int> bounds);

    void updatePositionFromMouse(const juce::MouseEvent& event);
    void updateSizeFromMouse(const juce::MouseEvent& event);

    AudioFileLoader& audioFileLoader;

    std::atomic<float>* positionParameter = nullptr;
    std::atomic<float>* grainSizeParameter = nullptr;
    double sourceSampleRate = 44100.0;
    int sourceLengthSamples = 0;

    // Mouse drag state
    enum class DragMode { None, LeftHandle, RightHandle, ClickPosition };
    DragMode dragMode = DragMode::None;

    // VBlank sync for display refresh rate
    juce::VBlankAttachment vBlankAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

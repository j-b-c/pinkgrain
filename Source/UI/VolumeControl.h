#pragma once

#include <JuceHeader.h>

class VolumeControl : public juce::Component,
                      public juce::Timer
{
public:
    VolumeControl();
    ~VolumeControl() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void timerCallback() override;

    juce::Slider& getSlider() { return volumeSlider; }

    void pushSamples(const float* leftChannel, const float* rightChannel, int numSamples);

private:
    void updateVolumeFromMouse(const juce::MouseEvent& event);

    juce::Slider volumeSlider;

    std::atomic<float> currentLevel { 0.0f };
    float displayLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolumeControl)
};

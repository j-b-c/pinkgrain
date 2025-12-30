#pragma once

#include <JuceHeader.h>

class ADSRControl : public juce::Component
{
public:
    ADSRControl();
    ~ADSRControl() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    juce::Slider& getAttackSlider() { return attackSlider; }
    juce::Slider& getDecaySlider() { return decaySlider; }
    juce::Slider& getSustainSlider() { return sustainSlider; }
    juce::Slider& getReleaseSlider() { return releaseSlider; }

private:
    void updateSliderFromMouse(juce::Slider& slider, const juce::MouseEvent& event, juce::Rectangle<int> bounds);
    void drawBar(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<int> bounds, const juce::String& label);

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRControl)
};

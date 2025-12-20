#pragma once

#include <JuceHeader.h>

class CustomDial : public juce::Component
{
public:
    CustomDial(const juce::String& labelText);
    ~CustomDial() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getSlider() { return slider; }

    void setLabelText(const juce::String& text);

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::Label valueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDial)
};

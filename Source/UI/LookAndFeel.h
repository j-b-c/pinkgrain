#pragma once

#include <JuceHeader.h>

class PinkGrainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PinkGrainLookAndFeel();
    ~PinkGrainLookAndFeel() override;

    // Colors
    static const juce::Colour backgroundColour;
    static const juce::Colour primaryColour;
    static const juce::Colour secondaryColour;
    static const juce::Colour textColour;
    static const juce::Colour darkAccentColour;

    // Rotary slider
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    // Toggle button
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // Text button
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    // Label
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinkGrainLookAndFeel)
};

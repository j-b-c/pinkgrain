#include "CustomDial.h"
#include "LookAndFeel.h"

CustomDial::CustomDial(const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);

    nameLabel.setText(labelText, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, PinkGrainLookAndFeel::textColour);
    nameLabel.setFont(juce::FontOptions(11.0f));
    addAndMakeVisible(nameLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, PinkGrainLookAndFeel::textColour);
    valueLabel.setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(valueLabel);

    slider.onValueChange = [this]()
    {
        valueLabel.setText(slider.getTextFromValue(slider.getValue()), juce::dontSendNotification);
    };
}

CustomDial::~CustomDial()
{
}

void CustomDial::resized()
{
    auto bounds = getLocalBounds();

    const int labelHeight = 16;
    const int valueHeight = 14;
    const int padding = 2;

    nameLabel.setBounds(bounds.removeFromTop(labelHeight));
    valueLabel.setBounds(bounds.removeFromBottom(valueHeight));
    bounds.removeFromBottom(padding);

    slider.setBounds(bounds);
}

void CustomDial::paint(juce::Graphics& /*g*/)
{
    // Background is handled by parent
}

void CustomDial::setLabelText(const juce::String& text)
{
    nameLabel.setText(text, juce::dontSendNotification);
}

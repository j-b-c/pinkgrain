#include "ADSRControl.h"
#include "LookAndFeel.h"

ADSRControl::ADSRControl()
{
    // Attack: 0-100ms
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    attackSlider.setRange(0.0, 100.0, 0.1);
    attackSlider.setValue(10.0);
    addChildComponent(attackSlider);

    // Decay: 0-500ms
    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    decaySlider.setRange(0.0, 500.0, 1.0);
    decaySlider.setValue(50.0);
    addChildComponent(decaySlider);

    // Sustain: 0-100%
    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sustainSlider.setRange(0.0, 1.0, 0.01);
    sustainSlider.setValue(0.8);
    addChildComponent(sustainSlider);

    // Release: 0-5000ms
    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    releaseSlider.setRange(0.0, 5000.0, 1.0);
    releaseSlider.setValue(50.0);
    addChildComponent(releaseSlider);
}

ADSRControl::~ADSRControl()
{
}

void ADSRControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    int barWidth = bounds.getWidth() / 4;

    drawBar(g, attackSlider, bounds.removeFromLeft(barWidth).reduced(2), "A");
    drawBar(g, decaySlider, bounds.removeFromLeft(barWidth).reduced(2), "D");
    drawBar(g, sustainSlider, bounds.removeFromLeft(barWidth).reduced(2), "S");
    drawBar(g, releaseSlider, bounds.removeFromLeft(barWidth).reduced(2), "R");
}

void ADSRControl::drawBar(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<int> bounds, const juce::String& label)
{
    auto barBounds = bounds.toFloat();

    // Background
    g.setColour(PinkGrainLookAndFeel::backgroundColour.brighter(0.1f));
    g.fillRoundedRectangle(barBounds, 3.0f);

    // Border
    g.setColour(PinkGrainLookAndFeel::primaryColour.darker(0.3f));
    g.drawRoundedRectangle(barBounds.reduced(0.5f), 3.0f, 1.0f);

    auto innerBounds = barBounds.reduced(2.0f);

    // Value fill (from bottom up)
    float normalizedValue = static_cast<float>((slider.getValue() - slider.getMinimum()) /
                                                (slider.getMaximum() - slider.getMinimum()));
    float fillHeight = innerBounds.getHeight() * normalizedValue;
    float fillY = innerBounds.getBottom() - fillHeight;

    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.6f));
    g.fillRoundedRectangle(innerBounds.getX(), fillY,
                           innerBounds.getWidth(), fillHeight, 2.0f);

    // Label at bottom
    g.setColour(PinkGrainLookAndFeel::textColour);
    g.setFont(11.0f);
    auto labelBounds = bounds.removeFromBottom(15);
    g.drawText(label, labelBounds, juce::Justification::centred);

    // Value display in the middle
    g.setFont(10.0f);
    juce::String valueText;
    float value = static_cast<float>(slider.getValue());

    if (label == "S")
    {
        // Sustain is 0-1, show as percentage
        valueText = juce::String(static_cast<int>(value * 100)) + "%";
    }
    else
    {
        // Time values
        if (value >= 1000.0f)
            valueText = juce::String(value / 1000.0f, 2) + "s";
        else
            valueText = juce::String(value, 0) + "ms";
    }

    auto valueBounds = bounds.reduced(2);
    g.drawText(valueText, valueBounds, juce::Justification::centred);
}

void ADSRControl::resized()
{
}

void ADSRControl::mouseDown(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds();
    int barWidth = bounds.getWidth() / 4;

    auto attackBounds = bounds.removeFromLeft(barWidth);
    auto decayBounds = bounds.removeFromLeft(barWidth);
    auto sustainBounds = bounds.removeFromLeft(barWidth);
    auto releaseBounds = bounds.removeFromLeft(barWidth);

    if (attackBounds.contains(event.getPosition()))
        updateSliderFromMouse(attackSlider, event, attackBounds);
    else if (decayBounds.contains(event.getPosition()))
        updateSliderFromMouse(decaySlider, event, decayBounds);
    else if (sustainBounds.contains(event.getPosition()))
        updateSliderFromMouse(sustainSlider, event, sustainBounds);
    else if (releaseBounds.contains(event.getPosition()))
        updateSliderFromMouse(releaseSlider, event, releaseBounds);
}

void ADSRControl::mouseDrag(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds();
    int barWidth = bounds.getWidth() / 4;

    auto attackBounds = bounds.removeFromLeft(barWidth);
    auto decayBounds = bounds.removeFromLeft(barWidth);
    auto sustainBounds = bounds.removeFromLeft(barWidth);
    auto releaseBounds = bounds.removeFromLeft(barWidth);

    if (attackBounds.contains(event.getMouseDownPosition()))
        updateSliderFromMouse(attackSlider, event, attackBounds);
    else if (decayBounds.contains(event.getMouseDownPosition()))
        updateSliderFromMouse(decaySlider, event, decayBounds);
    else if (sustainBounds.contains(event.getMouseDownPosition()))
        updateSliderFromMouse(sustainSlider, event, sustainBounds);
    else if (releaseBounds.contains(event.getMouseDownPosition()))
        updateSliderFromMouse(releaseSlider, event, releaseBounds);
}

void ADSRControl::updateSliderFromMouse(juce::Slider& slider, const juce::MouseEvent& event, juce::Rectangle<int> bounds)
{
    bounds = bounds.reduced(2);
    float y = static_cast<float>(event.y - bounds.getY());
    float height = static_cast<float>(bounds.getHeight() - 15); // Account for label

    // Invert (top = max, bottom = min)
    float normalizedValue = 1.0f - juce::jlimit(0.0f, 1.0f, y / height);
    float value = static_cast<float>(slider.getMinimum() + normalizedValue * (slider.getMaximum() - slider.getMinimum()));

    slider.setValue(value, juce::sendNotificationSync);
    repaint();
}

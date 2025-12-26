#include "VolumeControl.h"
#include "LookAndFeel.h"

VolumeControl::VolumeControl()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8);
    addChildComponent(volumeSlider); // Hidden, just for value storage

    startTimerHz(30);
}

VolumeControl::~VolumeControl()
{
    stopTimer();
}

void VolumeControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(PinkGrainLookAndFeel::backgroundColour.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border
    g.setColour(PinkGrainLookAndFeel::primaryColour.darker(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    auto innerBounds = bounds.reduced(2.0f);

    // Volume setting (darker pink fill)
    float volumeValue = static_cast<float>(volumeSlider.getValue());
    float volumeWidth = innerBounds.getWidth() * volumeValue;

    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.4f));
    g.fillRoundedRectangle(innerBounds.getX(), innerBounds.getY(),
                           volumeWidth, innerBounds.getHeight(), 2.0f);

    // Live level meter (lighter pink, on top)
    float meterWidth = innerBounds.getWidth() * displayLevel * volumeValue;
    if (meterWidth > 0.0f)
    {
        g.setColour(PinkGrainLookAndFeel::secondaryColour.withAlpha(0.8f));
        g.fillRoundedRectangle(innerBounds.getX(), innerBounds.getY(),
                               meterWidth, innerBounds.getHeight(), 2.0f);
    }

    // Volume percentage text
    g.setColour(PinkGrainLookAndFeel::textColour);
    g.setFont(12.0f);
    juce::String volumeText = juce::String(static_cast<int>(volumeValue * 100)) + "%";
    g.drawText(volumeText, bounds.toNearestInt(), juce::Justification::centred);
}

void VolumeControl::resized()
{
}

void VolumeControl::mouseDown(const juce::MouseEvent& event)
{
    updateVolumeFromMouse(event);
}

void VolumeControl::mouseDrag(const juce::MouseEvent& event)
{
    updateVolumeFromMouse(event);
}

void VolumeControl::updateVolumeFromMouse(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().reduced(2);
    float x = static_cast<float>(event.x - bounds.getX());
    float width = static_cast<float>(bounds.getWidth());

    float newValue = juce::jlimit(0.0f, 1.0f, x / width);
    volumeSlider.setValue(newValue, juce::sendNotificationSync);
    repaint();
}

void VolumeControl::timerCallback()
{
    // Smooth decay for the level meter
    float target = currentLevel.load();
    if (target > displayLevel)
    {
        displayLevel = target;
    }
    else
    {
        displayLevel = displayLevel * 0.85f + target * 0.15f;
    }

    // Reset current level (will be updated by next audio callback)
    currentLevel.store(0.0f);

    repaint();
}

void VolumeControl::pushSamples(const float* leftChannel, const float* rightChannel, int numSamples)
{
    float maxLevel = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float monoSample = (std::abs(leftChannel[i]) + std::abs(rightChannel[i])) * 0.5f;
        maxLevel = std::max(maxLevel, monoSample);
    }

    // Update atomic level (keep maximum)
    float expected = currentLevel.load();
    while (maxLevel > expected && !currentLevel.compare_exchange_weak(expected, maxLevel))
    {
    }
}

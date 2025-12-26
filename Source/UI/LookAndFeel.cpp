#include "LookAndFeel.h"

const juce::Colour PinkGrainLookAndFeel::backgroundColour = juce::Colour(0xFF0D0D0D);
const juce::Colour PinkGrainLookAndFeel::primaryColour = juce::Colour(0xFFFF1493);
const juce::Colour PinkGrainLookAndFeel::secondaryColour = juce::Colour(0xFFFF69B4);
const juce::Colour PinkGrainLookAndFeel::textColour = juce::Colour(0xFFFF1493);  // Same as primary
const juce::Colour PinkGrainLookAndFeel::darkAccentColour = juce::Colour(0xFF1A1A1A);

PinkGrainLookAndFeel::PinkGrainLookAndFeel()
{
    // Set default sans-serif font
    setDefaultSansSerifTypefaceName("Helvetica");

    setColour(juce::ResizableWindow::backgroundColourId, backgroundColour);
    setColour(juce::Label::textColourId, textColour);
    setColour(juce::TextButton::buttonColourId, darkAccentColour);
    setColour(juce::TextButton::textColourOffId, textColour);
    setColour(juce::TextButton::textColourOnId, primaryColour);
    setColour(juce::ComboBox::backgroundColourId, darkAccentColour);
    setColour(juce::ComboBox::textColourId, textColour);
    setColour(juce::ComboBox::outlineColourId, primaryColour);
}

PinkGrainLookAndFeel::~PinkGrainLookAndFeel()
{
}

void PinkGrainLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& /*slider*/)
{
    const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background circle (dark)
    g.setColour(darkAccentColour);
    g.fillEllipse(rx, ry, rw, rw);

    // Track arc (dark pink background)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(darkAccentColour.brighter(0.1f));
    g.strokePath(backgroundArc, juce::PathStrokeType(3.0f));

    // Value arc (pink)
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f,
                            0.0f, rotaryStartAngle, angle, true);
    g.setColour(primaryColour);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f));

    // Pointer line
    juce::Path pointer;
    const float pointerLength = radius * 0.6f;
    const float pointerThickness = 3.0f;
    pointer.addRectangle(-pointerThickness * 0.5f, -radius + 8.0f, pointerThickness, pointerLength);
    g.setColour(primaryColour);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // Centre dot
    g.setColour(primaryColour);
    g.fillEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f);
}

void PinkGrainLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool /*shouldDrawButtonAsDown*/)
{
    const float fontSize = juce::jmin(15.0f, static_cast<float>(button.getHeight()) * 0.75f);
    const float tickWidth = fontSize * 1.1f;

    juce::Rectangle<float> tickBounds(4.0f, (static_cast<float>(button.getHeight()) - tickWidth) * 0.5f,
                                       tickWidth, tickWidth);

    // Box background
    g.setColour(darkAccentColour);
    g.fillRoundedRectangle(tickBounds, 4.0f);

    // Box outline
    g.setColour(shouldDrawButtonAsHighlighted ? secondaryColour : primaryColour.darker(0.3f));
    g.drawRoundedRectangle(tickBounds, 4.0f, 1.5f);

    // Check mark
    if (button.getToggleState())
    {
        g.setColour(primaryColour);
        const float checkThickness = 2.5f;

        juce::Path checkPath;
        const float boxX = tickBounds.getX();
        const float boxY = tickBounds.getY();
        const float boxW = tickBounds.getWidth();
        const float boxH = tickBounds.getHeight();

        checkPath.startNewSubPath(boxX + boxW * 0.25f, boxY + boxH * 0.5f);
        checkPath.lineTo(boxX + boxW * 0.4f, boxY + boxH * 0.7f);
        checkPath.lineTo(boxX + boxW * 0.75f, boxY + boxH * 0.3f);

        g.strokePath(checkPath, juce::PathStrokeType(checkThickness));
    }

    // Label text
    g.setColour(textColour);
    g.setFont(fontSize);

    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds().withTrimmedLeft(static_cast<int>(tickWidth) + 10)
                                            .withTrimmedRight(2),
                     juce::Justification::centredLeft, 10);
}

void PinkGrainLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour& /*backgroundColour*/,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    juce::Colour fillColour = darkAccentColour;
    if (shouldDrawButtonAsDown)
        fillColour = primaryColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        fillColour = darkAccentColour.brighter(0.1f);

    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(primaryColour);
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
}

void PinkGrainLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        const juce::Font font(getLabelFont(label));
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(textArea.getHeight()) / font.getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

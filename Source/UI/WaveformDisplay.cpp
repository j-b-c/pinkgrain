#include "WaveformDisplay.h"
#include "LookAndFeel.h"

WaveformDisplay::WaveformDisplay(AudioFileLoader& loader, GrainEngine& /*engine*/)
    : audioFileLoader(loader),
      vBlankAttachment(this, [this] { onVBlank(); })
{
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(PinkGrainLookAndFeel::backgroundColour);
    g.fillRect(bounds);

    // Border
    g.setColour(PinkGrainLookAndFeel::primaryColour.darker(0.5f));
    g.drawRect(bounds, 1);

    auto innerBounds = bounds.reduced(2);

    if (audioFileLoader.hasFile())
    {
        drawWaveform(g, innerBounds);
        drawGrainWindow(g, innerBounds);
    }
    else
    {
        // No file loaded message
        g.setColour(PinkGrainLookAndFeel::textColour.withAlpha(0.5f));
        g.setFont(16.0f);
        g.drawText("Drop a WAV file or click Load", innerBounds, juce::Justification::centred);
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::onVBlank()
{
    // Update source info from audio file loader
    if (audioFileLoader.hasFile())
    {
        sourceSampleRate = audioFileLoader.getSampleRate();
        sourceLengthSamples = audioFileLoader.getNumSamples();
    }

    repaint();
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (!audioFileLoader.hasFile() || positionParameter == nullptr || grainSizeParameter == nullptr)
        return;

    auto bounds = getLocalBounds().reduced(2);
    if (bounds.isEmpty())
        return;

    float position = positionParameter->load();
    float grainSizeMs = grainSizeParameter->load();

    // Calculate grain window boundaries
    double fileLengthMs = (static_cast<double>(sourceLengthSamples) / sourceSampleRate) * 1000.0;
    float windowWidth = static_cast<float>(grainSizeMs / fileLengthMs);
    windowWidth = juce::jlimit(0.001f, 1.0f, windowWidth);

    float windowStart = position;
    float windowEnd = juce::jmin(1.0f, windowStart + windowWidth);

    float startX = bounds.getX() + windowStart * bounds.getWidth();
    float endX = bounds.getX() + windowEnd * bounds.getWidth();

    // Check if clicking near handles (within 8 pixels)
    const float handleTolerance = 8.0f;
    float mouseX = static_cast<float>(event.x);

    if (std::abs(mouseX - startX) < handleTolerance)
    {
        dragMode = DragMode::LeftHandle;
    }
    else if (std::abs(mouseX - endX) < handleTolerance)
    {
        dragMode = DragMode::RightHandle;
    }
    else
    {
        dragMode = DragMode::ClickPosition;
        updatePositionFromMouse(event);
    }
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (dragMode == DragMode::LeftHandle)
    {
        updatePositionFromMouse(event);
    }
    else if (dragMode == DragMode::RightHandle)
    {
        updateSizeFromMouse(event);
    }
    else if (dragMode == DragMode::ClickPosition)
    {
        updatePositionFromMouse(event);
    }
}

void WaveformDisplay::mouseUp(const juce::MouseEvent& /*event*/)
{
    dragMode = DragMode::None;
}

void WaveformDisplay::updatePositionFromMouse(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().reduced(2);
    if (bounds.isEmpty())
        return;

    float x = static_cast<float>(event.x - bounds.getX());
    float width = static_cast<float>(bounds.getWidth());

    float newPosition = juce::jlimit(0.0f, 1.0f, x / width);

    if (onPositionChanged)
    {
        onPositionChanged(newPosition);
    }
}

void WaveformDisplay::updateSizeFromMouse(const juce::MouseEvent& event)
{
    if (positionParameter == nullptr || sourceLengthSamples <= 0 || sourceSampleRate <= 0.0)
        return;

    auto bounds = getLocalBounds().reduced(2);
    if (bounds.isEmpty())
        return;

    float position = positionParameter->load();
    float x = static_cast<float>(event.x - bounds.getX());
    float width = static_cast<float>(bounds.getWidth());

    // Calculate normalized end position
    float normalizedEnd = juce::jlimit(0.0f, 1.0f, x / width);

    // Calculate window width from position to end
    float windowWidth = normalizedEnd - position;
    windowWidth = juce::jlimit(0.001f, 1.0f - position, windowWidth);

    // Convert to grain size in milliseconds
    double fileLengthMs = (static_cast<double>(sourceLengthSamples) / sourceSampleRate) * 1000.0;
    float newSizeMs = static_cast<float>(windowWidth * fileLengthMs);

    // Clamp to valid range (10ms to 30000ms based on the parameter range)
    newSizeMs = juce::jlimit(10.0f, 30000.0f, newSizeMs);

    if (onSizeChanged)
    {
        onSizeChanged(newSizeMs);
    }
}

void WaveformDisplay::setPositionParameter(std::atomic<float>* positionParam)
{
    positionParameter = positionParam;
}

void WaveformDisplay::setGrainSizeParameter(std::atomic<float>* grainSizeParam)
{
    grainSizeParameter = grainSizeParam;
}

void WaveformDisplay::setSourceSampleRate(double sampleRate)
{
    sourceSampleRate = sampleRate;
}

void WaveformDisplay::setSourceLengthSamples(int lengthSamples)
{
    sourceLengthSamples = lengthSamples;
}

void WaveformDisplay::drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto& thumbnail = audioFileLoader.getThumbnail();

    if (thumbnail.getNumChannels() == 0)
        return;

    // Draw waveform in a dimmer color
    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.4f));
    thumbnail.drawChannels(g, bounds, 0.0, thumbnail.getTotalLength(), 1.0f);
}

void WaveformDisplay::drawGrainWindow(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (positionParameter == nullptr || grainSizeParameter == nullptr)
        return;

    if (sourceLengthSamples <= 0 || sourceSampleRate <= 0.0)
        return;

    float position = positionParameter->load();
    float grainSizeMs = grainSizeParameter->load();

    // Calculate grain window size as a proportion of the total file
    double fileLengthMs = (static_cast<double>(sourceLengthSamples) / sourceSampleRate) * 1000.0;
    float windowWidth = static_cast<float>(grainSizeMs / fileLengthMs);
    windowWidth = juce::jlimit(0.001f, 1.0f, windowWidth);

    // Calculate window position (centered on position, but clamped to bounds)
    float windowStart = position;
    float windowEnd = juce::jmin(1.0f, windowStart + windowWidth);

    // Convert to pixel coordinates
    float startX = bounds.getX() + windowStart * bounds.getWidth();
    float endX = bounds.getX() + windowEnd * bounds.getWidth();
    float windowPixelWidth = endX - startX;

    // Draw the grain window as a shaded rectangle
    juce::Rectangle<float> windowRect(startX, static_cast<float>(bounds.getY()),
                                        windowPixelWidth, static_cast<float>(bounds.getHeight()));

    // Semi-transparent pink overlay for the window
    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.15f));
    g.fillRect(windowRect);

    // Brighter waveform within the window
    auto& thumbnail = audioFileLoader.getThumbnail();
    if (thumbnail.getNumChannels() > 0)
    {
        g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.7f));

        // Calculate time range for the window
        double totalLength = thumbnail.getTotalLength();
        double startTime = position * totalLength;
        double endTime = (position + windowWidth) * totalLength;

        // Clip to visible area
        g.reduceClipRegion(windowRect.toNearestInt());
        thumbnail.drawChannels(g, bounds, startTime, endTime, 1.0f);
        g.resetToDefaultState();

        // Restore clip region for rest of painting
        g.reduceClipRegion(getLocalBounds());
    }

    // Draw drag handles as thicker, more visible rectangles
    const float handleWidth = 4.0f;
    g.setColour(PinkGrainLookAndFeel::primaryColour);

    // Left handle (position)
    juce::Rectangle<float> leftHandle(startX - handleWidth * 0.5f, static_cast<float>(bounds.getY()),
                                       handleWidth, static_cast<float>(bounds.getHeight()));
    g.fillRect(leftHandle);

    // Right handle (size)
    juce::Rectangle<float> rightHandle(endX - handleWidth * 0.5f, static_cast<float>(bounds.getY()),
                                        handleWidth, static_cast<float>(bounds.getHeight()));
    g.fillRect(rightHandle);
}


#include "WaveformDisplay.h"
#include "LookAndFeel.h"

WaveformDisplay::WaveformDisplay(AudioFileLoader& loader, GrainEngine& engine)
    : audioFileLoader(loader),
      grainEngine(engine)
{
    startTimerHz(60);  // 60 FPS for smooth grain animations
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
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
        drawGrains(g, innerBounds);
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

void WaveformDisplay::timerCallback()
{
    // Update source info from audio file loader
    if (audioFileLoader.hasFile())
    {
        sourceSampleRate = audioFileLoader.getSampleRate();
        sourceLengthSamples = audioFileLoader.getNumSamples();
    }

    // Update cached grain info for thread-safe rendering
    cachedGrainInfo = grainEngine.getActiveGrainInfo();
    repaint();
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

    // Draw window borders
    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.8f));
    g.drawVerticalLine(static_cast<int>(startX), static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
    g.drawVerticalLine(static_cast<int>(endX), static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
}

void WaveformDisplay::drawGrains(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (cachedGrainInfo.empty())
        return;

    const int numGrains = static_cast<int>(cachedGrainInfo.size());
    const float availableWidth = static_cast<float>(bounds.getWidth());

    // Determine dot size - fixed at 3x3 pixels
    const float dotSize = 3.0f;
    const int maxVisibleDots = static_cast<int>(availableWidth / (dotSize + 1.0f));

    int grainsPerDot = 1;
    if (numGrains > maxVisibleDots)
    {
        grainsPerDot = (numGrains + maxVisibleDots - 1) / maxVisibleDots;
    }

    // Draw dots for grains
    for (int i = 0; i < numGrains; i += grainsPerDot)
    {
        // Average properties if consolidating multiple grains
        float avgPosition = 0.0f;
        float avgEnvelope = 0.0f;
        int count = 0;

        for (size_t j = static_cast<size_t>(i); j < static_cast<size_t>(juce::jmin(i + grainsPerDot, numGrains)); ++j)
        {
            const auto& grain = cachedGrainInfo[j];
            if (grain.active)
            {
                // Calculate position within the grain window based on progress
                float grainWindowSize = grain.grainEndPosition - grain.grainStartPosition;
                float posInWindow = grain.grainStartPosition + grain.grainProgress * grainWindowSize;
                avgPosition += posInWindow;
                avgEnvelope += grain.envelopeLevel;
                ++count;
            }
        }

        if (count == 0)
            continue;

        avgPosition /= static_cast<float>(count);
        avgEnvelope /= static_cast<float>(count);

        // Calculate dot position
        float x = bounds.getX() + avgPosition * bounds.getWidth();

        // Vertical position: spread dots across the height with some randomness based on index
        float yNormalized = 0.5f + 0.3f * std::sin(static_cast<float>(i) * 0.7f);
        float y = bounds.getY() + yNormalized * bounds.getHeight();

        // Alpha based on envelope level
        float alpha = avgEnvelope * 0.9f + 0.1f;

        // Draw 3x3 dot
        g.setColour(PinkGrainLookAndFeel::secondaryColour.withAlpha(alpha));
        g.fillRect(x - dotSize * 0.5f, y - dotSize * 0.5f, dotSize, dotSize);
    }
}

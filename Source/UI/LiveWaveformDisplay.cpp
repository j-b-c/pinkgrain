#include "LiveWaveformDisplay.h"
#include "LookAndFeel.h"

LiveWaveformDisplay::LiveWaveformDisplay()
{
    leftBuffer.fill(0.0f);
    rightBuffer.fill(0.0f);
    startTimerHz(60);
}

LiveWaveformDisplay::~LiveWaveformDisplay()
{
    stopTimer();
}

void LiveWaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(PinkGrainLookAndFeel::backgroundColour);
    g.fillRect(bounds);

    // Border
    g.setColour(PinkGrainLookAndFeel::primaryColour.darker(0.5f));
    g.drawRect(bounds, 1);

    auto innerBounds = bounds.reduced(2);

    // Draw center line
    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.2f));
    g.drawHorizontalLine(innerBounds.getCentreY(),
                         static_cast<float>(innerBounds.getX()),
                         static_cast<float>(innerBounds.getRight()));

    // Draw waveform
    juce::ScopedLock lock(pathLock);
    if (!waveformPath.isEmpty())
    {
        g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.8f));
        g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
    }
}

void LiveWaveformDisplay::resized()
{
    updateWaveformPath();
}

void LiveWaveformDisplay::timerCallback()
{
    updateWaveformPath();
    repaint();
}

void LiveWaveformDisplay::pushSamples(const float* leftChannel, const float* rightChannel, int numSamples)
{
    int pos = writePosition.load();

    for (int i = 0; i < numSamples; ++i)
    {
        leftBuffer[pos] = leftChannel[i];
        rightBuffer[pos] = rightChannel[i];
        pos = (pos + 1) % BUFFER_SIZE;
    }

    writePosition.store(pos);
}

void LiveWaveformDisplay::updateWaveformPath()
{
    auto bounds = getLocalBounds().reduced(2);
    if (bounds.isEmpty())
        return;

    const float width = static_cast<float>(bounds.getWidth());
    const float height = static_cast<float>(bounds.getHeight());
    const float centreY = bounds.getCentreY();

    juce::Path newPath;

    int readPos = writePosition.load();
    const int samplesToDisplay = juce::jmin(BUFFER_SIZE, static_cast<int>(width));
    const int startPos = (readPos - samplesToDisplay + BUFFER_SIZE) % BUFFER_SIZE;

    // Calculate samples per pixel
    const float samplesPerPixel = static_cast<float>(samplesToDisplay) / width;

    bool pathStarted = false;

    for (int x = 0; x < static_cast<int>(width); ++x)
    {
        // Get sample index for this pixel
        int sampleIndex = (startPos + static_cast<int>(x * samplesPerPixel)) % BUFFER_SIZE;

        // Mix left and right channels
        float sample = (leftBuffer[sampleIndex] + rightBuffer[sampleIndex]) * 0.5f;

        // Clamp sample
        sample = juce::jlimit(-1.0f, 1.0f, sample);

        // Calculate y position
        float y = centreY - sample * (height * 0.45f);

        if (!pathStarted)
        {
            newPath.startNewSubPath(static_cast<float>(bounds.getX() + x), y);
            pathStarted = true;
        }
        else
        {
            newPath.lineTo(static_cast<float>(bounds.getX() + x), y);
        }
    }

    juce::ScopedLock lock(pathLock);
    waveformPath = newPath;
}

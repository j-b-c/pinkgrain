#include "ZoomedWaveformDisplay.h"
#include "LookAndFeel.h"

ZoomedWaveformDisplay::ZoomedWaveformDisplay(AudioFileLoader& loader, GrainEngine& engine)
    : audioFileLoader(loader),
      grainEngine(engine),
      vBlankAttachment(this, [this] { onVBlank(); })
{
}

ZoomedWaveformDisplay::~ZoomedWaveformDisplay()
{
}

void ZoomedWaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(PinkGrainLookAndFeel::backgroundColour);
    g.fillRect(bounds);

    // Border
    g.setColour(PinkGrainLookAndFeel::primaryColour.darker(0.5f));
    g.drawRect(bounds, 1);

    auto innerBounds = bounds.reduced(2);

    if (audioFileLoader.hasFile() && positionParameter != nullptr && grainSizeParameter != nullptr)
    {
        drawZoomedWaveform(g, innerBounds);
        drawGrains(g, innerBounds);
    }
    else
    {
        g.setColour(PinkGrainLookAndFeel::textColour.withAlpha(0.3f));
        g.setFont(14.0f);
        g.drawText("Grain Window (zoomed)", innerBounds, juce::Justification::centred);
    }
}

void ZoomedWaveformDisplay::resized()
{
}

void ZoomedWaveformDisplay::onVBlank()
{
    cachedGrainInfo = grainEngine.getActiveGrainInfo();
    repaint();
}

void ZoomedWaveformDisplay::setPositionParameter(std::atomic<float>* positionParam)
{
    positionParameter = positionParam;
}

void ZoomedWaveformDisplay::setGrainSizeParameter(std::atomic<float>* grainSizeParam)
{
    grainSizeParameter = grainSizeParam;
}

void ZoomedWaveformDisplay::drawZoomedWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto& thumbnail = audioFileLoader.getThumbnail();

    if (thumbnail.getNumChannels() == 0)
        return;

    float position = positionParameter->load();
    float grainSizeMs = grainSizeParameter->load();

    double sourceSampleRate = audioFileLoader.getSampleRate();
    int sourceLengthSamples = audioFileLoader.getNumSamples();

    if (sourceLengthSamples <= 0 || sourceSampleRate <= 0.0)
        return;

    // Calculate grain window in time
    double fileLengthSeconds = static_cast<double>(sourceLengthSamples) / sourceSampleRate;
    double grainSizeSeconds = grainSizeMs / 1000.0;

    double startTime = position * fileLengthSeconds;
    double endTime = startTime + grainSizeSeconds;
    endTime = juce::jmin(endTime, fileLengthSeconds);

    // Draw the zoomed waveform
    g.setColour(PinkGrainLookAndFeel::primaryColour.withAlpha(0.7f));
    thumbnail.drawChannels(g, bounds, startTime, endTime, 1.0f);
}

void ZoomedWaveformDisplay::drawGrains(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (cachedGrainInfo.empty())
        return;

    float position = positionParameter->load();
    float grainSizeMs = grainSizeParameter->load();

    double sourceSampleRate = audioFileLoader.getSampleRate();
    int sourceLengthSamples = audioFileLoader.getNumSamples();

    if (sourceLengthSamples <= 0 || sourceSampleRate <= 0.0)
        return;

    // Calculate the window being displayed (in normalized 0-1 file coordinates)
    double fileLengthMs = (static_cast<double>(sourceLengthSamples) / sourceSampleRate) * 1000.0;
    float windowWidth = static_cast<float>(grainSizeMs / fileLengthMs);
    windowWidth = juce::jlimit(0.001f, 1.0f, windowWidth);

    float windowStart = position;
    float windowEnd = juce::jmin(1.0f, windowStart + windowWidth);

    const float dotSize = 3.0f;

    // Draw a dot for each active grain
    for (size_t i = 0; i < cachedGrainInfo.size(); ++i)
    {
        const auto& grain = cachedGrainInfo[i];
        if (!grain.active)
            continue;

        // normalizedPosition is the actual current read position in the file (0-1)
        // This automatically handles reverse playback since the position moves backwards
        float filePos = grain.normalizedPosition;

        // Map the file position to the zoomed window coordinates
        // windowStart maps to x=0, windowEnd maps to x=width
        float normalizedInWindow = (filePos - windowStart) / (windowEnd - windowStart);

        // Skip grains that are outside the visible window
        if (normalizedInWindow < -0.1f || normalizedInWindow > 1.1f)
            continue;

        float x = bounds.getX() + normalizedInWindow * static_cast<float>(bounds.getWidth());

        // Vertical position: spread dots across the height based on grain index
        float yNormalized = 0.5f + 0.35f * std::sin(static_cast<float>(i) * 2.3f);
        float y = bounds.getY() + yNormalized * static_cast<float>(bounds.getHeight());

        // Alpha based on envelope level - this shows attack/release
        float alpha = grain.envelopeLevel * 0.9f + 0.1f;

        // Color based on MIDI note (lower notes = darker, higher notes = lighter)
        // Map MIDI range 36-96 (5 octaves) to brightness range
        int midiNote = grain.midiNote;
        float brightness = juce::jlimit(0.0f, 1.0f, (static_cast<float>(midiNote - 36) / 60.0f));

        // Interpolate between dark and light color based on note
        juce::Colour grainColour = PinkGrainLookAndFeel::secondaryColour.interpolatedWith(
            juce::Colours::white, brightness * 0.6f);

        // Draw 3x3 dot
        g.setColour(grainColour.withAlpha(alpha));
        g.fillRect(x - dotSize * 0.5f, y - dotSize * 0.5f, dotSize, dotSize);
    }
}

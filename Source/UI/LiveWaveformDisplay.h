#pragma once

#include <JuceHeader.h>

class LiveWaveformDisplay : public juce::Component,
                            public juce::Timer
{
public:
    LiveWaveformDisplay();
    ~LiveWaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Call from audio thread to push samples
    void pushSamples(const float* leftChannel, const float* rightChannel, int numSamples);

private:
    static constexpr int BUFFER_SIZE = 2048;

    std::array<float, BUFFER_SIZE> leftBuffer;
    std::array<float, BUFFER_SIZE> rightBuffer;
    std::atomic<int> writePosition { 0 };

    // Cached waveform path for rendering
    juce::Path waveformPath;
    juce::CriticalSection pathLock;

    void updateWaveformPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LiveWaveformDisplay)
};

#pragma once

#include <JuceHeader.h>

class AudioFileLoader
{
public:
    AudioFileLoader();
    ~AudioFileLoader();

    bool loadFile(const juce::File& file);
    void clear();

    bool hasFile() const { return fileLoaded; }
    const juce::AudioBuffer<float>& getBuffer() const { return audioBuffer; }
    double getSampleRate() const { return sampleRate; }
    int getNumChannels() const { return audioBuffer.getNumChannels(); }
    int getNumSamples() const { return audioBuffer.getNumSamples(); }
    double getLengthInSeconds() const;

    juce::String getFileName() const { return fileName; }

    // For waveform display
    juce::AudioThumbnail& getThumbnail() { return thumbnail; }
    juce::AudioThumbnailCache& getThumbnailCache() { return thumbnailCache; }

    // Listener for file load events
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void fileLoaded(const juce::String& fileName) = 0;
        virtual void fileCleared() = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> audioBuffer;
    double sampleRate = 44100.0;
    bool fileLoaded = false;
    juce::String fileName;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileLoader)
};

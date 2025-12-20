#include "AudioFileLoader.h"

AudioFileLoader::AudioFileLoader()
    : thumbnailCache(5),
      thumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
}

AudioFileLoader::~AudioFileLoader()
{
}

bool AudioFileLoader::loadFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
        return false;

    // Read the entire file into the buffer
    const int numSamples = static_cast<int>(reader->lengthInSamples);
    const int numChannels = static_cast<int>(reader->numChannels);

    audioBuffer.setSize(numChannels, numSamples);
    reader->read(&audioBuffer, 0, numSamples, 0, true, true);

    sampleRate = reader->sampleRate;
    fileName = file.getFileName();
    fileLoaded = true;

    // Update thumbnail
    thumbnail.setSource(new juce::FileInputSource(file));

    // Notify listeners
    listeners.call([this](Listener& l) { l.fileLoaded(fileName); });

    return true;
}

void AudioFileLoader::clear()
{
    audioBuffer.setSize(0, 0);
    sampleRate = 44100.0;
    fileLoaded = false;
    fileName = "";
    thumbnail.clear();

    listeners.call([](Listener& l) { l.fileCleared(); });
}

double AudioFileLoader::getLengthInSeconds() const
{
    if (!fileLoaded || sampleRate <= 0.0)
        return 0.0;

    return static_cast<double>(audioBuffer.getNumSamples()) / sampleRate;
}

void AudioFileLoader::addListener(Listener* listener)
{
    listeners.add(listener);
}

void AudioFileLoader::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

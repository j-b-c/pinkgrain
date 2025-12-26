#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/LiveWaveformDisplay.h"
#include "UI/VolumeControl.h"

// Parameter IDs
const juce::String PinkGrainAudioProcessor::GRAIN_SIZE_ID = "grainSize";
const juce::String PinkGrainAudioProcessor::DENSITY_ID = "density";
const juce::String PinkGrainAudioProcessor::POSITION_ID = "position";
const juce::String PinkGrainAudioProcessor::PITCH_ID = "pitch";
const juce::String PinkGrainAudioProcessor::PAN_SPREAD_ID = "panSpread";
const juce::String PinkGrainAudioProcessor::ATTACK_ID = "attack";
const juce::String PinkGrainAudioProcessor::RELEASE_ID = "release";
const juce::String PinkGrainAudioProcessor::REVERSE_ID = "reverse";
const juce::String PinkGrainAudioProcessor::SPRAY_ID = "spray";
const juce::String PinkGrainAudioProcessor::PITCH_RANDOM_ID = "pitchRandom";
const juce::String PinkGrainAudioProcessor::VOLUME_ID = "volume";

PinkGrainAudioProcessor::PinkGrainAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    restoreSession();
}

PinkGrainAudioProcessor::~PinkGrainAudioProcessor()
{
    saveSession();
}

juce::AudioProcessorValueTreeState::ParameterLayout PinkGrainAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(GRAIN_SIZE_ID, 1),
        "Grain Size",
        juce::NormalisableRange<float>(10.0f, 30000.0f, 1.0f, 0.3f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value >= 1000.0f)
                return juce::String(value / 1000.0f, 2) + " s";
            return juce::String(value, 0) + " ms";
        },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DENSITY_ID, 1),
        "Density",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.5f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " g/s"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(POSITION_ID, 1),
        "Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PITCH_ID, 1),
        "Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " st"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PAN_SPREAD_ID, 1),
        "Pan Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ATTACK_ID, 1),
        "Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 0.5f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(RELEASE_ID, 1),
        "Release",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f, 0.5f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(REVERSE_ID, 1),
        "Reverse",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(SPRAY_ID, 1),
        "Spray",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PITCH_RANDOM_ID, 1),
        "Pitch Random",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " st"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(VOLUME_ID, 1),
        "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.8f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; },
        nullptr));

    return { params.begin(), params.end() };
}

const juce::String PinkGrainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PinkGrainAudioProcessor::acceptsMidi() const
{
    return true;
}

bool PinkGrainAudioProcessor::producesMidi() const
{
    return false;
}

bool PinkGrainAudioProcessor::isMidiEffect() const
{
    return false;
}

double PinkGrainAudioProcessor::getTailLengthSeconds() const
{
    return 0.5;  // Account for grain release time
}

int PinkGrainAudioProcessor::getNumPrograms()
{
    return 1;
}

int PinkGrainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PinkGrainAudioProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String PinkGrainAudioProcessor::getProgramName(int /*index*/)
{
    return {};
}

void PinkGrainAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void PinkGrainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    grainEngine.prepare(sampleRate, samplesPerBlock);

    if (audioFileLoader.hasFile())
    {
        grainEngine.setSourceBuffer(&audioFileLoader.getBuffer(), audioFileLoader.getSampleRate());
    }
}

void PinkGrainAudioProcessor::releaseResources()
{
}

bool PinkGrainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void PinkGrainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear output buffer
    buffer.clear();

    // Update grain engine parameters from APVTS
    updateGrainEngineParameters();

    // Update source buffer if file is loaded
    if (audioFileLoader.hasFile())
    {
        grainEngine.setSourceBuffer(&audioFileLoader.getBuffer(), audioFileLoader.getSampleRate());
    }

    // Process MIDI messages
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            grainEngine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        }
        else if (msg.isNoteOff())
        {
            grainEngine.noteOff(msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            grainEngine.allNotesOff();
        }
    }

    // Process grains
    grainEngine.process(buffer);

    // Push samples to live waveform display
    if (liveWaveformDisplay != nullptr && buffer.getNumChannels() >= 2)
    {
        liveWaveformDisplay->pushSamples(buffer.getReadPointer(0),
                                          buffer.getReadPointer(1),
                                          buffer.getNumSamples());
    }

    // Push samples to volume control for level metering
    if (volumeControl != nullptr && buffer.getNumChannels() >= 2)
    {
        volumeControl->pushSamples(buffer.getReadPointer(0),
                                   buffer.getReadPointer(1),
                                   buffer.getNumSamples());
    }
}

void PinkGrainAudioProcessor::updateGrainEngineParameters()
{
    grainEngine.setGrainSize(*apvts.getRawParameterValue(GRAIN_SIZE_ID));
    grainEngine.setDensity(*apvts.getRawParameterValue(DENSITY_ID));
    grainEngine.setPosition(*apvts.getRawParameterValue(POSITION_ID));
    grainEngine.setPitch(*apvts.getRawParameterValue(PITCH_ID));
    grainEngine.setPanSpread(*apvts.getRawParameterValue(PAN_SPREAD_ID));
    grainEngine.setAttack(*apvts.getRawParameterValue(ATTACK_ID));
    grainEngine.setRelease(*apvts.getRawParameterValue(RELEASE_ID));
    grainEngine.setReverse(*apvts.getRawParameterValue(REVERSE_ID) > 0.5f);
    grainEngine.setSpray(*apvts.getRawParameterValue(SPRAY_ID));
    grainEngine.setPitchRandom(*apvts.getRawParameterValue(PITCH_RANDOM_ID));
    grainEngine.setVolume(*apvts.getRawParameterValue(VOLUME_ID));
}

bool PinkGrainAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PinkGrainAudioProcessor::createEditor()
{
    return new PinkGrainAudioProcessorEditor(*this);
}

void PinkGrainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    // Add file path as a property
    state.setProperty("audioFilePath", currentFilePath, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PinkGrainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            auto newState = juce::ValueTree::fromXml(*xmlState);

            // Extract and restore file path
            juce::String filePath = newState.getProperty("audioFilePath", "").toString();
            if (filePath.isNotEmpty())
            {
                juce::File file(filePath);
                if (file.existsAsFile())
                {
                    audioFileLoader.loadFile(file);
                    currentFilePath = filePath;
                }
            }

            apvts.replaceState(newState);
        }
    }
}

void PinkGrainAudioProcessor::setCurrentFilePath(const juce::String& path)
{
    currentFilePath = path;
}

juce::File PinkGrainAudioProcessor::getPresetsDirectory() const
{
    juce::File presetDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                              .getChildFile("PinkGrain")
                              .getChildFile("Presets");
    presetDir.createDirectory();
    return presetDir;
}

juce::StringArray PinkGrainAudioProcessor::getPresetList() const
{
    juce::StringArray presets;
    juce::File presetDir = getPresetsDirectory();

    for (const auto& file : presetDir.findChildFiles(juce::File::findFiles, false, "*.xml"))
    {
        presets.add(file.getFileNameWithoutExtension());
    }

    presets.sort(true);
    return presets;
}

void PinkGrainAudioProcessor::savePreset(const juce::String& presetName)
{
    juce::File presetFile = getPresetsDirectory().getChildFile(presetName + ".xml");

    juce::MemoryBlock data;
    getStateInformation(data);

    presetFile.replaceWithData(data.getData(), data.getSize());
}

void PinkGrainAudioProcessor::loadPreset(const juce::String& presetName)
{
    juce::File presetFile = getPresetsDirectory().getChildFile(presetName + ".xml");

    if (presetFile.existsAsFile())
    {
        juce::MemoryBlock data;
        if (presetFile.loadFileAsData(data))
        {
            setStateInformation(data.getData(), static_cast<int>(data.getSize()));
        }
    }
}

juce::File PinkGrainAudioProcessor::getSessionFile() const
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("PinkGrain");
    appDataDir.createDirectory();
    return appDataDir.getChildFile("lastSession.xml");
}

void PinkGrainAudioProcessor::saveSession()
{
    juce::File sessionFile = getSessionFile();

    juce::MemoryBlock data;
    getStateInformation(data);

    sessionFile.replaceWithData(data.getData(), data.getSize());
}

void PinkGrainAudioProcessor::restoreSession()
{
    juce::File sessionFile = getSessionFile();

    if (sessionFile.existsAsFile())
    {
        juce::MemoryBlock data;
        if (sessionFile.loadFileAsData(data))
        {
            setStateInformation(data.getData(), static_cast<int>(data.getSize()));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PinkGrainAudioProcessor();
}

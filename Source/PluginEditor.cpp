#include "PluginProcessor.h"
#include "PluginEditor.h"

PinkGrainAudioProcessorEditor::PinkGrainAudioProcessorEditor(PinkGrainAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      waveformDisplay(p.getAudioFileLoader(), p.getGrainEngine()),
      zoomedWaveformDisplay(p.getAudioFileLoader(), p.getGrainEngine()),
      sizeDial("SIZE"),
      densityDial("DENSITY"),
      positionDial("POSITION"),
      pitchDial("PITCH"),
      panDial("PAN"),
      sprayDial("SPRAY"),
      pitchRandomDial("PITCH RND"),
      maxGrainsDial("MAX GRAINS")
{
    setLookAndFeel(&lookAndFeel);

    // Header components
    loadFileButton.setButtonText("Load");
    loadFileButton.onClick = [this]() { loadFileButtonClicked(); };
    addAndMakeVisible(loadFileButton);

    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this]() { savePresetButtonClicked(); };
    addAndMakeVisible(savePresetButton);

    presetCombo.setTextWhenNothingSelected("Presets...");
    presetCombo.onChange = [this]() { presetComboChanged(); };
    addAndMakeVisible(presetCombo);
    refreshPresetList();

    titleLabel.setText("PINKGRAIN", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, PinkGrainLookAndFeel::primaryColour);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(volumeControl);
    audioProcessor.setVolumeControl(&volumeControl);

    // Waveform displays
    addAndMakeVisible(waveformDisplay);
    waveformDisplay.setPositionParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::POSITION_ID));
    waveformDisplay.setGrainSizeParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::GRAIN_SIZE_ID));

    addAndMakeVisible(liveWaveformDisplay);
    audioProcessor.setLiveWaveformDisplay(&liveWaveformDisplay);

    // Zoomed waveform display
    addAndMakeVisible(zoomedWaveformDisplay);
    zoomedWaveformDisplay.setPositionParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::POSITION_ID));
    zoomedWaveformDisplay.setGrainSizeParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::GRAIN_SIZE_ID));

    // Set up mouse drag callback to update position parameter
    waveformDisplay.onPositionChanged = [this](float newPosition)
    {
        if (auto* param = audioProcessor.getApvts().getParameter(PinkGrainAudioProcessor::POSITION_ID))
        {
            param->setValueNotifyingHost(param->convertTo0to1(newPosition));
        }
    };

    // Set up mouse drag callback to update size parameter
    waveformDisplay.onSizeChanged = [this](float newSizeMs)
    {
        if (auto* param = audioProcessor.getApvts().getParameter(PinkGrainAudioProcessor::GRAIN_SIZE_ID))
        {
            param->setValueNotifyingHost(param->convertTo0to1(newSizeMs));
        }
    };

    // Row 1 dials
    addAndMakeVisible(sizeDial);
    addAndMakeVisible(densityDial);
    addAndMakeVisible(positionDial);
    addAndMakeVisible(pitchDial);
    addAndMakeVisible(panDial);
    addAndMakeVisible(sprayDial);

    // Row 2 - ADSR control
    addAndMakeVisible(adsrControl);

    reverseButton.setButtonText("REVERSE");
    reverseButton.setColour(juce::ToggleButton::textColourId, PinkGrainLookAndFeel::textColour);
    reverseButton.setColour(juce::ToggleButton::tickColourId, PinkGrainLookAndFeel::primaryColour);
    addAndMakeVisible(reverseButton);

    addAndMakeVisible(pitchRandomDial);
    addAndMakeVisible(maxGrainsDial);

    // Parameter attachments
    auto& apvts = audioProcessor.getApvts();

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::VOLUME_ID, volumeControl.getSlider());

    sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::GRAIN_SIZE_ID, sizeDial.getSlider());

    densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::DENSITY_ID, densityDial.getSlider());

    positionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::POSITION_ID, positionDial.getSlider());

    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::PITCH_ID, pitchDial.getSlider());

    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::PAN_SPREAD_ID, panDial.getSlider());

    sprayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::SPRAY_ID, sprayDial.getSlider());

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::ATTACK_ID, adsrControl.getAttackSlider());

    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::DECAY_ID, adsrControl.getDecaySlider());

    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::SUSTAIN_ID, adsrControl.getSustainSlider());

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::RELEASE_ID, adsrControl.getReleaseSlider());

    reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, PinkGrainAudioProcessor::REVERSE_ID, reverseButton);

    pitchRandomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::PITCH_RANDOM_ID, pitchRandomDial.getSlider());

    maxGrainsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::MAX_GRAINS_ID, maxGrainsDial.getSlider());

    setSize(800, 600);
}

PinkGrainAudioProcessorEditor::~PinkGrainAudioProcessorEditor()
{
    audioProcessor.setLiveWaveformDisplay(nullptr);
    audioProcessor.setVolumeControl(nullptr);
    setLookAndFeel(nullptr);
}

void PinkGrainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(PinkGrainLookAndFeel::backgroundColour);
}

void PinkGrainAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Header row
    auto headerRow = bounds.removeFromTop(50);
    loadFileButton.setBounds(headerRow.removeFromLeft(70).reduced(0, 10));
    headerRow.removeFromLeft(5);
    savePresetButton.setBounds(headerRow.removeFromLeft(70).reduced(0, 10));
    headerRow.removeFromLeft(5);
    presetCombo.setBounds(headerRow.removeFromLeft(150).reduced(0, 10));
    headerRow.removeFromLeft(10);

    auto volumeArea = headerRow.removeFromRight(200);
    volumeControl.setBounds(volumeArea.reduced(0, 12));

    titleLabel.setBounds(headerRow);

    bounds.removeFromTop(10);

    // Waveform displays - side by side
    auto waveformArea = bounds.removeFromTop(140);
    const int gap = 10;
    const int liveWidth = 200;
    auto liveArea = waveformArea.removeFromRight(liveWidth);
    waveformArea.removeFromRight(gap);
    waveformDisplay.setBounds(waveformArea);
    liveWaveformDisplay.setBounds(liveArea);

    bounds.removeFromTop(5);

    // Zoomed waveform display - full width below
    auto zoomedArea = bounds.removeFromTop(80);
    zoomedWaveformDisplay.setBounds(zoomedArea);

    bounds.removeFromTop(10);

    // Parameter dials - Row 1
    auto row1 = bounds.removeFromTop(90);
    const int dialWidth = row1.getWidth() / 6;

    sizeDial.setBounds(row1.removeFromLeft(dialWidth));
    densityDial.setBounds(row1.removeFromLeft(dialWidth));
    positionDial.setBounds(row1.removeFromLeft(dialWidth));
    pitchDial.setBounds(row1.removeFromLeft(dialWidth));
    panDial.setBounds(row1.removeFromLeft(dialWidth));
    sprayDial.setBounds(row1.removeFromLeft(dialWidth));

    bounds.removeFromTop(10);

    // Parameter dials - Row 2
    // Use remaining height for ADSR (taller bars)
    auto row2 = bounds;

    // ADSR control on left (about 200px wide)
    adsrControl.setBounds(row2.removeFromLeft(200));
    row2.removeFromLeft(20);

    // Remaining space split for reverse button, pitch random, and max grains (use top 90px)
    auto controlsArea = row2.removeFromTop(90);
    const int remainingWidth = controlsArea.getWidth() / 3;

    auto reverseArea = controlsArea.removeFromLeft(remainingWidth);
    reverseButton.setBounds(reverseArea.reduced(20, 30));

    pitchRandomDial.setBounds(controlsArea.removeFromLeft(remainingWidth));
    maxGrainsDial.setBounds(controlsArea.removeFromLeft(remainingWidth));
}

void PinkGrainAudioProcessorEditor::loadFileButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select an audio file to load...",
        juce::File{},
        "*.wav;*.aif;*.aiff;*.mp3;*.flac");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            audioProcessor.getAudioFileLoader().loadFile(file);
            audioProcessor.setCurrentFilePath(file.getFullPathName());
        }
    });
}

void PinkGrainAudioProcessorEditor::savePresetButtonClicked()
{
    auto presetName = std::make_unique<juce::AlertWindow>(
        "Save Preset",
        "Enter a name for the preset:",
        juce::MessageBoxIconType::NoIcon);

    presetName->addTextEditor("name", "", "Preset Name:");
    presetName->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    presetName->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    presetName->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alertWindow = presetName.release()](int result)
        {
            std::unique_ptr<juce::AlertWindow> aw(alertWindow);
            if (result == 1)
            {
                juce::String name = aw->getTextEditorContents("name").trim();
                if (name.isNotEmpty())
                {
                    audioProcessor.savePreset(name);
                    refreshPresetList();
                }
            }
        }));
}

void PinkGrainAudioProcessorEditor::presetComboChanged()
{
    int selectedId = presetCombo.getSelectedId();
    if (selectedId > 0)
    {
        juce::String presetName = presetCombo.getItemText(presetCombo.getSelectedItemIndex());
        audioProcessor.loadPreset(presetName);
    }
}

void PinkGrainAudioProcessorEditor::refreshPresetList()
{
    presetCombo.clear(juce::dontSendNotification);

    juce::StringArray presets = audioProcessor.getPresetList();
    int id = 1;
    for (const auto& preset : presets)
    {
        presetCombo.addItem(preset, id++);
    }
}

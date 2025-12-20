#include "PluginProcessor.h"
#include "PluginEditor.h"

PinkGrainAudioProcessorEditor::PinkGrainAudioProcessorEditor(PinkGrainAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      volumeDial("VOLUME"),
      waveformDisplay(p.getAudioFileLoader(), p.getGrainEngine()),
      sizeDial("SIZE"),
      densityDial("DENSITY"),
      positionDial("POSITION"),
      pitchDial("PITCH"),
      panDial("PAN"),
      sprayDial("SPRAY"),
      attackDial("ATTACK"),
      releaseDial("RELEASE"),
      pitchRandomDial("PITCH RND")
{
    setLookAndFeel(&lookAndFeel);

    // Header components
    loadFileButton.setButtonText("Load File");
    loadFileButton.onClick = [this]() { loadFileButtonClicked(); };
    addAndMakeVisible(loadFileButton);

    titleLabel.setText("PINKGRAIN", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, PinkGrainLookAndFeel::primaryColour);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(volumeDial);

    // Waveform display
    addAndMakeVisible(waveformDisplay);
    waveformDisplay.setPositionParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::POSITION_ID));
    waveformDisplay.setGrainSizeParameter(audioProcessor.getApvts().getRawParameterValue(PinkGrainAudioProcessor::GRAIN_SIZE_ID));

    // Row 1 dials
    addAndMakeVisible(sizeDial);
    addAndMakeVisible(densityDial);
    addAndMakeVisible(positionDial);
    addAndMakeVisible(pitchDial);
    addAndMakeVisible(panDial);
    addAndMakeVisible(sprayDial);

    // Row 2 dials
    addAndMakeVisible(attackDial);
    addAndMakeVisible(releaseDial);

    reverseButton.setButtonText("REVERSE");
    reverseButton.setColour(juce::ToggleButton::textColourId, PinkGrainLookAndFeel::textColour);
    reverseButton.setColour(juce::ToggleButton::tickColourId, PinkGrainLookAndFeel::primaryColour);
    addAndMakeVisible(reverseButton);

    addAndMakeVisible(pitchRandomDial);

    // Parameter attachments
    auto& apvts = audioProcessor.getApvts();

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::VOLUME_ID, volumeDial.getSlider());

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
        apvts, PinkGrainAudioProcessor::ATTACK_ID, attackDial.getSlider());

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::RELEASE_ID, releaseDial.getSlider());

    reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, PinkGrainAudioProcessor::REVERSE_ID, reverseButton);

    pitchRandomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, PinkGrainAudioProcessor::PITCH_RANDOM_ID, pitchRandomDial.getSlider());

    setSize(800, 500);
}

PinkGrainAudioProcessorEditor::~PinkGrainAudioProcessorEditor()
{
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
    loadFileButton.setBounds(headerRow.removeFromLeft(100));
    headerRow.removeFromLeft(10);

    auto volumeArea = headerRow.removeFromRight(70);
    volumeDial.setBounds(volumeArea);

    titleLabel.setBounds(headerRow);

    bounds.removeFromTop(10);

    // Waveform display
    auto waveformArea = bounds.removeFromTop(180);
    waveformDisplay.setBounds(waveformArea);

    bounds.removeFromTop(15);

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
    auto row2 = bounds.removeFromTop(90);
    const int row2Width = row2.getWidth() / 4;

    attackDial.setBounds(row2.removeFromLeft(row2Width));
    releaseDial.setBounds(row2.removeFromLeft(row2Width));

    auto reverseArea = row2.removeFromLeft(row2Width);
    reverseButton.setBounds(reverseArea.reduced(20, 30));

    pitchRandomDial.setBounds(row2.removeFromLeft(row2Width));
}

void PinkGrainAudioProcessorEditor::loadFileButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select a WAV file to load...",
        juce::File{},
        "*.wav;*.aif;*.aiff;*.mp3;*.flac");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            audioProcessor.getAudioFileLoader().loadFile(file);
        }
    });
}

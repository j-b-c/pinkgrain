#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/CustomDial.h"
#include "UI/WaveformDisplay.h"
#include "UI/LiveWaveformDisplay.h"

class PinkGrainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PinkGrainAudioProcessorEditor(PinkGrainAudioProcessor&);
    ~PinkGrainAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void loadFileButtonClicked();

    PinkGrainAudioProcessor& audioProcessor;

    PinkGrainLookAndFeel lookAndFeel;

    // Header
    juce::TextButton loadFileButton;
    juce::Label titleLabel;
    CustomDial volumeDial;

    // Waveform displays
    WaveformDisplay waveformDisplay;
    LiveWaveformDisplay liveWaveformDisplay;

    // Parameter dials - Row 1
    CustomDial sizeDial;
    CustomDial densityDial;
    CustomDial positionDial;
    CustomDial pitchDial;
    CustomDial panDial;
    CustomDial sprayDial;

    // Parameter dials - Row 2
    CustomDial attackDial;
    CustomDial releaseDial;
    juce::ToggleButton reverseButton;
    CustomDial pitchRandomDial;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> positionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sprayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchRandomAttachment;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinkGrainAudioProcessorEditor)
};

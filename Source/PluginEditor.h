#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ParamEqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ParamEqAudioProcessorEditor (ParamEqAudioProcessor&);
    ~ParamEqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParamEqAudioProcessor& audioProcessor;

    // Componentes GUI
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> freqSliders;
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> gainSliders;
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> qSliders;
    juce::ComboBox filterTypeSelector1; // Seletor de filtro para banda 1
    juce::ComboBox filterTypeSelector8; // Seletor de filtro para banda 8

    // Analisador de espectro
    std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer;

    // Attachments para os sliders
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> freqAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> gainAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> qAttachments;

    // Attachments para os ComboBoxes de tipo de filtro
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment1;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessorEditor)
};

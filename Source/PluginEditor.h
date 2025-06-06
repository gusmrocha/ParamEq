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


    // Conex�es entre GUI e par�metros (SliderAttachment)
    // - Mant�m sincronia autom�tica
    // - Gerencia listeners
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> freqAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> gainAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> qAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessorEditor)
};

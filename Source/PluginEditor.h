/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

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
    juce::Slider freqSlider, gainSlider, qSlider;  // Controles deslizantes

    // Conex�es entre GUI e par�metros (SliderAttachment)
    // - Mant�m sincronia autom�tica
    // - Gerencia listeners
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach, gainAttach, qAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessorEditor)
};

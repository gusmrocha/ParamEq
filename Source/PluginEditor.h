#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
*/

// Classe de estilo visual personalizada
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Cores globais suaves
        setColour(juce::Slider::thumbColourId, juce::Colours::skyblue);
        setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
        setColour(juce::Slider::backgroundColourId, juce::Colours::darkslategrey);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::grey.darker());
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background
        g.setColour(juce::Colours::dimgrey);
        g.fillEllipse(rx, ry, rw, rw);

        // Outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        // Thumb
        juce::Path p;
        auto thumbLength = radius * 0.6f;
        auto thumbThickness = 2.0f;
        p.addRectangle(-thumbThickness * 0.5f, -radius, thumbThickness, thumbLength);

        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = 4.0f;

        juce::Rectangle<float> track(x + width * 0.5f - trackWidth * 0.5f, y, trackWidth, height);
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRect(track);

        juce::Rectangle<float> thumbRect(x + width * 0.5f - 6.0f, sliderPos - 6.0f, 12.0f, 12.0f);
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbRect);
    }
};


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

    // Look and Feel personalizado para o eq
    CustomLookAndFeel customLNF;

    // Componentes GUI
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> freqSliders;
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> gainSliders;
    std::array<juce::Slider, ParamEqAudioProcessor::NUM_BANDS> qSliders;
    juce::ComboBox filterTypeSelector1; // Seletor de filtro para banda 1
    juce::ComboBox filterTypeSelector8; // Seletor de filtro para banda 8

    // Label para os controles
    std::array<juce::Label, ParamEqAudioProcessor::NUM_BANDS> freqLabels;
    std::array<juce::Label, ParamEqAudioProcessor::NUM_BANDS> gainLabels;
    std::array<juce::Label, ParamEqAudioProcessor::NUM_BANDS> qLabels;


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

// ParamEQ - Parametric Equalizer Plugin
// Copyright (C) 2025 Gustavo Mugnol Rocha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
        // Configurações globais existentes (mantidas)
        setColour(juce::Slider::thumbColourId, juce::Colours::skyblue);
        setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
        setColour(juce::Slider::backgroundColourId, juce::Colours::darkslategrey);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::grey.darker());

        // Esquema de cores por banda (novo)
        bandColors = {
            juce::Colour(255, 100, 100),   // Banda 1: Vermelho
            juce::Colour(255, 180, 100),   // Banda 2: Laranja
            juce::Colour(255, 255, 100),   // Banda 3: Amarelo
            juce::Colour(180, 255, 100),   // Banda 4: Verde-limão
            juce::Colour(100, 255, 100),   // Banda 5: Verde
            juce::Colour(100, 255, 255),   // Banda 6: Ciano
            juce::Colour(100, 180, 255),   // Banda 7: Azul-claro
            juce::Colour(180, 100, 255)    // Banda 8: Roxo
        };
    }

    // Mantém os métodos drawRotarySlider e drawLinearSlider existentes
    // apenas adicionando a lógica de cores

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

        // Fundo - usando cor da banda com opacidade reduzida
        g.setColour(getBandColor().withAlpha(0.3f));
        g.fillEllipse(rx, ry, rw, rw);

        // Contorno
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        // Ponteiro
        juce::Path p;
        auto thumbLength = radius * 0.6f;
        p.addRectangle(-2.0f, -radius, 4.0f, thumbLength);
        
        g.setColour(getBandColor().brighter(0.2f));
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = 4.0f;
        juce::Rectangle<float> track(x + width * 0.5f - trackWidth * 0.5f, y, trackWidth, height);
        
        // Trilha - cor da banda com opacidade
        g.setColour(getBandColor().withAlpha(0.4f));
        g.fillRect(track);

        // Thumb
        juce::Rectangle<float> thumbRect(x + width * 0.5f - 6.0f, sliderPos - 6.0f, 12.0f, 12.0f);
        g.setColour(getBandColor());
        g.fillEllipse(thumbRect);
    }

    void setCurrentBand(int band) { currentBand = band % bandColors.size(); }
    juce::Colour getBandColor() const { return bandColors[currentBand]; }

    juce::Colour getBandColor(int band) const { 
        return bandColors[band % bandColors.size()]; 
    }
    
    void updateBandColors(const std::vector<juce::Colour>& newColors) {
        bandColors = newColors;
    }

private:
    std::vector<juce::Colour> bandColors;
    int currentBand = 0;
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
    std::array<juce::ComboBox, ParamEqAudioProcessor::NUM_BANDS> filterTypeSelectors;

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
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> typeAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessorEditor)
};

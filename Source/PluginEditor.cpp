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


#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParamEqAudioProcessorEditor::ParamEqAudioProcessorEditor (ParamEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    const int numBands = ParamEqAudioProcessor::NUM_BANDS;

    for (int band = 0; band < numBands; ++band)
    {
        // === ComboBox de tipo de filtro ===
        auto& combo = filterTypeSelectors[band];
        combo.addItemList({"Peak", "Low Shelf", "High Shelf", "Low Pass", "High Pass"}, 1);
        combo.setColour(juce::ComboBox::backgroundColourId, customLNF.getBandColor(band).withAlpha(0.2f));
        combo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        addAndMakeVisible(combo);

        typeAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.parameters, "TYPE" + juce::String(band + 1), combo));

        // === Sliders ===
        auto& freq = freqSliders[band];
        auto& gain = gainSliders[band];
        auto& q = qSliders[band];

        freq.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        freq.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
        freq.setNumDecimalPlacesToDisplay(0);
        freq.setTextValueSuffix(" Hz");

        q.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        q.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
        q.setNumDecimalPlacesToDisplay(2);

        gain.setSliderStyle(juce::Slider::LinearVertical);
        gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
        gain.setTextValueSuffix(" dB");

        // Look and Feel
        freq.setLookAndFeel(&customLNF);
        gain.setLookAndFeel(&customLNF);
        q.setLookAndFeel(&customLNF);

        // === Labels ===
        auto& freqLabel = freqLabels[band];
        auto& qLabel = qLabels[band];
        auto& gainLabel = gainLabels[band];

        freqLabel.setText("Freq", juce::dontSendNotification);
        qLabel.setText("Q", juce::dontSendNotification);
        gainLabel.setText("Gain", juce::dontSendNotification);

        freqLabel.attachToComponent(&freq, false);
        qLabel.attachToComponent(&q, false);
        gainLabel.attachToComponent(&gain, false);

        freqLabel.setJustificationType(juce::Justification::centred);
        qLabel.setJustificationType(juce::Justification::centred);
        gainLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(freqLabel);
        addAndMakeVisible(qLabel);
        addAndMakeVisible(gainLabel);

        freqLabel.setColour(juce::Label::textColourId, customLNF.getBandColor(band));
        qLabel.setColour(juce::Label::textColourId, customLNF.getBandColor(band));
        gainLabel.setColour(juce::Label::textColourId, customLNF.getBandColor(band));

        // === Visibilidade ===
        addAndMakeVisible(freq);
        addAndMakeVisible(q);
        addAndMakeVisible(gain);

        // === Attachments ===
        freqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "FREQ" + juce::String(band + 1), freq));
        gainAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "GAIN" + juce::String(band + 1), gain));
        qAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "Q" + juce::String(band + 1), q));

        // === Cores ===
        customLNF.setCurrentBand(band);
        auto bandColor = customLNF.getBandColor();

        // Freq
        freq.setColour(juce::Slider::thumbColourId, bandColor);
        freq.setColour(juce::Slider::rotarySliderFillColourId, bandColor.withAlpha(0.7f));
        freq.setColour(juce::Slider::rotarySliderOutlineColourId, bandColor.darker(0.5f));
        freq.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);

        // Q
        q.setColour(juce::Slider::thumbColourId, bandColor);
        q.setColour(juce::Slider::rotarySliderFillColourId, bandColor.withAlpha(0.7f));
        q.setColour(juce::Slider::rotarySliderOutlineColourId, bandColor.darker(0.5f));
        q.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);

        // Gain
        gain.setColour(juce::Slider::thumbColourId, bandColor.brighter(0.2f));
        gain.setColour(juce::Slider::trackColourId, bandColor.withAlpha(0.4f));
        gain.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    }

    // === Analisador de espectro ===
    spectrumAnalyzer = std::make_unique<SpectrumAnalyzer>(audioProcessor);
    addAndMakeVisible(spectrumAnalyzer.get());
    audioProcessor.spectrumAnalyzer = spectrumAnalyzer.get();

    setSize(1000, 500);
}


ParamEqAudioProcessorEditor::~ParamEqAudioProcessorEditor() {
    audioProcessor.spectrumAnalyzer = nullptr; // Limpa o ponteiro do analisador de espectro
    
    // Limpa os Look and Feel para evitar vazamentos de memória
    for (int band = 0; band < ParamEqAudioProcessor::NUM_BANDS; ++band) {
    freqSliders[band].setLookAndFeel(nullptr);
    qSliders[band].setLookAndFeel(nullptr);
    gainSliders[band].setLookAndFeel(nullptr);
}
}

//==============================================================================
void ParamEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ParamEqAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // 1. Espectro - mantemos a altura atual
    const int spectrumHeight = 220;
    spectrumAnalyzer->setBounds(area.removeFromTop(spectrumHeight));

    const int numBands = ParamEqAudioProcessor::NUM_BANDS;
    const int bandSpacing = 6;
    const int bandWidth = (area.getWidth() - bandSpacing * (numBands - 1)) / numBands;

    // Aumentamos os espaçamentos e dimensões
    const int comboHeight = 25;
    const int knobSize = 60;
    const int knobSpacing = 6;
    const int gainWidth = 60;
    const int gainHeight = 90;
    const int labelHeight = 20;
    const int verticalSpacing = 10;

    // Área para os controles
    area.removeFromTop(15); // Espaço extra após o espectro

    for (int band = 0; band < numBands; ++band)
    {
        // Adiciona cores customizadas de acordo com o LookAndFeel
        customLNF.setCurrentBand(band);
        auto bandColor = customLNF.getBandColor();
        freqLabels[band].setColour(juce::Label::textColourId, bandColor);
        qLabels[band].setColour(juce::Label::textColourId, bandColor);
        gainLabels[band].setColour(juce::Label::textColourId, bandColor);


        int x = area.getX() + band * (bandWidth + bandSpacing);
        int y = area.getY();

        juce::Rectangle<int> bandArea(x, y, bandWidth, area.getHeight());

        // 1. ComboBox no topo (bandas 1 e 8)
        juce::Rectangle<int> comboArea = bandArea.removeFromTop(comboHeight + 2);
        filterTypeSelectors[band].setBounds(comboArea.reduced(2));


        // 2. Labels e knobs de frequência/Q
        int knobsTotalWidth = (2 * knobSize + knobSpacing);
        int knobsX = x + (bandWidth - knobsTotalWidth) / 2;
        
        // Posiciona labels acima dos knobs
        freqLabels[band].setBounds(knobsX, bandArea.getY(), knobSize, labelHeight);
        qLabels[band].setBounds(knobsX + knobSize + knobSpacing, bandArea.getY(), knobSize, labelHeight);
        
        // Posiciona knobs abaixo dos labels
        bandArea.removeFromTop(labelHeight + 2); // Espaço para os labels
        freqSliders[band].setBounds(knobsX, bandArea.getY(), knobSize, knobSize);
        qSliders[band].setBounds(knobsX + knobSize + knobSpacing, bandArea.getY(), knobSize, knobSize);

        // 3. Label e slider de gain (abaixo dos knobs)
        bandArea.removeFromTop(knobSize + verticalSpacing); // Espaço após knobs
        int gainX = x + (bandWidth - gainWidth) / 2;
        
        // Label do gain
        gainLabels[band].setBounds(gainX, bandArea.getY(), gainWidth, labelHeight);
        
        // Slider do gain (abaixo do label)
        bandArea.removeFromTop(labelHeight + 2);
        gainSliders[band].setBounds(gainX, bandArea.getY(), gainWidth, gainHeight);
    }
}
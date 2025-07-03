#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParamEqAudioProcessorEditor::ParamEqAudioProcessorEditor (ParamEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    const int numBands = ParamEqAudioProcessor::NUM_BANDS;

    // Configuração dos ComboBoxes para tipos de filtro
    filterTypeSelector1.addItemList({"Peak", "Low Shelf", "High Pass"}, 1);
    filterTypeSelector8.addItemList({"Peak", "High Shelf", "Low Pass"}, 1);
    
    addAndMakeVisible(filterTypeSelector1);
    addAndMakeVisible(filterTypeSelector8);
    
    typeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "TYPE1", filterTypeSelector1);
    typeAttachment8 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "TYPE8", filterTypeSelector8);

    for (int band = 0; band < numBands; band++) {
        // === Configuração dos Sliders ===
        // Frequência
        freqSliders[band].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        freqSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        freqSliders[band].setNumDecimalPlacesToDisplay(0);
        freqSliders[band].setTextValueSuffix(" Hz");

        // Q
        qSliders[band].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        qSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        qSliders[band].setNumDecimalPlacesToDisplay(2);

        // Gain
        gainSliders[band].setSliderStyle(juce::Slider::LinearVertical);
        gainSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        gainSliders[band].setTextValueSuffix(" dB");

        // === Visibilidade ===
        addAndMakeVisible(freqSliders[band]);
        addAndMakeVisible(qSliders[band]);
        addAndMakeVisible(gainSliders[band]);

        // === Attachments ===
        freqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "FREQ" + juce::String(band + 1), freqSliders[band]
        ));
        gainAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "GAIN" + juce::String(band + 1), gainSliders[band]
        ));
        qAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "Q" + juce::String(band + 1), qSliders[band]
        ));
    }

    spectrumAnalyzer = std::make_unique<SpectrumAnalyzer>(audioProcessor);
    addAndMakeVisible(spectrumAnalyzer.get());
    audioProcessor.spectrumAnalyzer = spectrumAnalyzer.get();

    setSize(1000, 600); // Aumente a altura para o espectro
}

ParamEqAudioProcessorEditor::~ParamEqAudioProcessorEditor() {
    audioProcessor.spectrumAnalyzer = nullptr; // Limpa o ponteiro do analisador de espectro
}

//==============================================================================
void ParamEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ParamEqAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // 1. Espectro maior
    const int spectrumHeight = 220;
    spectrumAnalyzer->setBounds(area.removeFromTop(spectrumHeight));

    const int numBands = ParamEqAudioProcessor::NUM_BANDS;
    const int bandSpacing = 6;
    const int bandWidth = (area.getWidth() - bandSpacing * (numBands - 1)) / numBands;

    const int comboHeight = 25;
    const int knobSize = 65;          // ⇦ aumentamos os knobs
    const int knobSpacing = 6;
    const int gainWidth = 34;
    const int gainHeight = 90;        // ⇦ reduzimos os sliders de ganho

    for (int band = 0; band < numBands; ++band)
    {
        int x = area.getX() + band * (bandWidth + bandSpacing);
        int y = area.getY();

        juce::Rectangle<int> bandArea(x, y, bandWidth, area.getHeight());

        // ComboBox no topo
        if (band == 0)
            filterTypeSelector1.setBounds(bandArea.removeFromTop(comboHeight).reduced(2));
        else if (band == 7)
            filterTypeSelector8.setBounds(bandArea.removeFromTop(comboHeight).reduced(2));
        else
            bandArea.removeFromTop(comboHeight + 2); // alinhamento consistente

        // Calcular área restante
        const int totalHeight = bandArea.getHeight();
        const int knobsY = bandArea.getY() + 5;

        // Centrando os knobs horizontalmente
        int knobsTotalWidth = (2 * knobSize + knobSpacing);
        int knobsX = x + (bandWidth - knobsTotalWidth) / 2;

        freqSliders[band].setBounds(knobsX, knobsY, knobSize, knobSize);
        qSliders[band].setBounds(knobsX + knobSize + knobSpacing, knobsY, knobSize, knobSize);

        // Gain: fixo embaixo, alinhado
        int gainX = x + (bandWidth - gainWidth) / 2;
        int gainY = bandArea.getBottom() - gainHeight;
        gainSliders[band].setBounds(gainX, gainY, gainWidth, gainHeight);
    }
}


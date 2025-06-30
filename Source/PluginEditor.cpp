#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParamEqAudioProcessorEditor::ParamEqAudioProcessorEditor (ParamEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    const int numBands = ParamEqAudioProcessor::NUM_BANDS;

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
        gainSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
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
    spectrumAnalyzer->setBounds(area.removeFromTop(200)); // 200px de altura

    const int numBands = ParamEqAudioProcessor::NUM_BANDS;
    const int bandMargin = 5;

    const int minBandWidth = 100;
    const int bandWidth = juce::jmax(minBandWidth, (area.getWidth() - (numBands - 1) * bandMargin) / numBands);

    for (int band = 0; band < numBands; band++) {
        auto bandArea = area.removeFromLeft(bandWidth);
        if (band < numBands - 1)
            area.removeFromLeft(bandMargin);

        // === Área dos knobs (superior) ===
        auto knobsArea = bandArea.removeFromTop(160);

        const int knobSize = 70;
        const int knobSpacing = 10;
        const int knobsTotalWidth = knobSize * 2 + knobSpacing;

        const int knobsStartX = knobsArea.getX() + (knobsArea.getWidth() - knobsTotalWidth) / 2;
        const int knobsY = knobsArea.getY() + (knobsArea.getHeight() - knobSize) / 2;

        freqSliders[band].setBounds(knobsStartX, knobsY, knobSize, knobSize);
        qSliders[band].setBounds(knobsStartX + knobSize + knobSpacing, knobsY, knobSize, knobSize);

        // === Área do slider de Gain (inferior) ===
        const int gainWidth = 40;
        const int gainHeight = 180;

        const int gainX = bandArea.getX() + (bandArea.getWidth() - gainWidth) / 2;
        const int gainY = bandArea.getY() + (bandArea.getHeight() - gainHeight) / 2;

        gainSliders[band].setBounds(gainX, gainY, gainWidth, gainHeight);
    }
}

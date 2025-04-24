/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParamEqAudioProcessorEditor::ParamEqAudioProcessorEditor (ParamEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //=== Configura��o dos Sliders ============================================
    // Slider rotativo para frequ�ncia
    freqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);  // Estilo "knob"
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);  // Caixa de texto embaixo

    // Slider linear vertical para ganho
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    // Slider rotativo para Q
    qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    // Adiciona todos os sliders � interface
    for (auto* slider : { &freqSlider, &gainSlider, &qSlider }) {
        addAndMakeVisible(slider);  // Torna vis�vel e adiciona ao layout
    }

    //=== Conex�o aos Par�metros ==============================================
    // Cria attachments que vinculam sliders aos par�metros
    freqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters,  // Refer�ncia ao ValueTreeState
        "FREQ",  // ID do par�metro
        freqSlider  // Slider correspondente
    );

    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters,
        "GAIN",
        gainSlider
    );

    qAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters,
        "Q",
        qSlider
    );

    // Tamanho inicial da janela
    setSize (400, 300);
}

ParamEqAudioProcessorEditor::~ParamEqAudioProcessorEditor()
{
}

//==============================================================================
void ParamEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));


}

void ParamEqAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);  // �rea �til com margem
    const int itemWidth = 80;  // Largura de cada controle

    // Posiciona frequ�ncia � esquerda
    freqSlider.setBounds(area.removeFromLeft(itemWidth));

    // Posiciona ganho (com redu��o vertical)
    gainSlider.setBounds(area.removeFromLeft(itemWidth).reduced(0, 10));

    // Posiciona Q
    qSlider.setBounds(area.removeFromLeft(itemWidth));
}

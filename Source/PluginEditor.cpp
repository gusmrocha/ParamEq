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
        freqSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
        freqSliders[band].setNumDecimalPlacesToDisplay(0);
        freqSliders[band].setTextValueSuffix(" Hz");

        // Q
        qSliders[band].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        qSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
        qSliders[band].setNumDecimalPlacesToDisplay(2);

        // Gain
        gainSliders[band].setSliderStyle(juce::Slider::LinearVertical);
        gainSliders[band].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
        gainSliders[band].setTextValueSuffix(" dB");

        // Look and Feel customizados
        freqSliders[band].setLookAndFeel(&customLNF);
        qSliders[band].setLookAndFeel(&customLNF);
        gainSliders[band].setLookAndFeel(&customLNF);

        // Aplica cores aos labels
        customLNF.setCurrentBand(band);
        freqLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());
        qLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());
        gainLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());    // Aplica cores aos labels
        customLNF.setCurrentBand(band);
        freqLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());
        qLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());
        gainLabels[band].setColour(juce::Label::textColourId, customLNF.getBandColor());


        // === Visibilidade ===
        addAndMakeVisible(freqSliders[band]);
        addAndMakeVisible(qSliders[band]);
        addAndMakeVisible(gainSliders[band]);

        // Configura labels de frequência
        freqLabels[band].setText("Freq", juce::dontSendNotification);
        freqLabels[band].attachToComponent(&freqSliders[band], false);
        freqLabels[band].setJustificationType(juce::Justification::centred);
        freqLabels[band].setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(freqLabels[band]);

        // Configura labels de Q
        qLabels[band].setText("Q", juce::dontSendNotification);
        qLabels[band].attachToComponent(&qSliders[band], false);
        qLabels[band].setJustificationType(juce::Justification::centred);
        qLabels[band].setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(qLabels[band]);

        // Configura labels de Gain
        gainLabels[band].setText("Gain", juce::dontSendNotification);
        gainLabels[band].attachToComponent(&gainSliders[band], false);
        gainLabels[band].setJustificationType(juce::Justification::centred);
        gainLabels[band].setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(gainLabels[band]);

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
        
        // Configuração final de cores
        customLNF.setCurrentBand(band);
        
        // Aplica estilos consistentes para todos os sliders
        auto bandColor = customLNF.getBandColor();
        
        // Sliders de frequência
        freqSliders[band].setColour(juce::Slider::thumbColourId, bandColor);
        freqSliders[band].setColour(juce::Slider::rotarySliderFillColourId, bandColor.withAlpha(0.7f));
        freqSliders[band].setColour(juce::Slider::rotarySliderOutlineColourId, bandColor.darker(0.5f));
        
        // Sliders de Q
        qSliders[band].setColour(juce::Slider::thumbColourId, bandColor);
        qSliders[band].setColour(juce::Slider::rotarySliderFillColourId, bandColor.withAlpha(0.7f));
        qSliders[band].setColour(juce::Slider::rotarySliderOutlineColourId, bandColor.darker(0.5f));
        
        // Sliders de ganho
        gainSliders[band].setColour(juce::Slider::thumbColourId, bandColor.brighter(0.2f));
        gainSliders[band].setColour(juce::Slider::trackColourId, bandColor.withAlpha(0.4f));
        
        // Textos
        freqSliders[band].setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        qSliders[band].setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        gainSliders[band].setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);

    }

    spectrumAnalyzer = std::make_unique<SpectrumAnalyzer>(audioProcessor);
    addAndMakeVisible(spectrumAnalyzer.get());
    audioProcessor.spectrumAnalyzer = spectrumAnalyzer.get();

    setSize(1000, 500); //
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

        if (band == 0)
            filterTypeSelector1.setBounds(comboArea.reduced(2));
        else if (band == 7)
            filterTypeSelector8.setBounds(comboArea.reduced(2));

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
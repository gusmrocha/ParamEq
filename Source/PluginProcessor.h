#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>


//==============================================================================
/**  Equalizador param�trico simples, com interface grafica
	e controle de par�metros via AudioProcessorValueTreeState.
	O filtro � um StateVariableTPTFilter, que � um filtro de
	segunda ordem com resposta em frequ�ncia ajust�vel.
	O plugin tem um editor gr�fico que permite ajustar os par�metros
	do filtro em tempo real. O editor � criado na classe
	ParamEqAudioProcessorEditor.

    Posteriormente, ser� adicionado uma janela para visualiza��o do
    espectro, e a curva de equaliza��o dos filtros.
*/
class ParamEqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ParamEqAudioProcessor();
    ~ParamEqAudioProcessor() override;

    //=============================== Reprodu��o de �udio (buffer) ======================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //=============================== Gera o editor ================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //============================= MIDI =======================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //===============================Programs (presets)========================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //============================Gerenciamento de estado do plugin(configs atuais)=================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Sistema de par�metros
    juce::AudioProcessorValueTreeState parameters;

    bool supportsDoublePrecisionProcessing() const override { return false; }
private:
    //====================================Defini��o do filtro==========================================
    juce::dsp::ProcessorDuplicator<  // duplica��o para os dois canais
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > filter;

    juce::dsp::ProcessSpec spec;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessor)
};

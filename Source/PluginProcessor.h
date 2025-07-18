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
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "SpectrumAnalyzer.h"


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

// Declaração antecipada para quebrar dependência circular
class SpectrumAnalyzer;

// Enumeração para os tipos de filtro
enum FilterType {
    PEAK,
    LOW_SHELF,
    HIGH_SHELF,
    LOW_PASS,
    HIGH_PASS
};

class ParamEqAudioProcessor  : public juce::AudioProcessor,
                               public juce::AudioProcessorParameter::Listener
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

    // Sistema de parâmetros
    juce::AudioProcessorValueTreeState parameters;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

    bool supportsDoublePrecisionProcessing() const override { return false; }
    static constexpr int NUM_BANDS = 8;
    static juce::String getFilterTypeName(FilterType type);

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int, bool) override {}  // não usado

    // Cache de coeficientes de filtro
    std::array<juce::dsp::IIR::Coefficients<float>::Ptr, NUM_BANDS> cachedCoefficients;
    std::array<std::atomic<bool>, NUM_BANDS> coefficientsDirty;
    void updateCachedCoefficients();



    // Tipos de filtro
    static FilterType getMappedFilterType(int choiceIndex)
    {
        switch (choiceIndex)
        {
            case 0:  return PEAK;
            case 1:  return LOW_SHELF;
            case 2:  return HIGH_SHELF;
            case 3:  return LOW_PASS;
            case 4:  return HIGH_PASS;
            default: return PEAK;
        }
    }

    // Espectro
    void pushBufferToAnalyzer(const juce::AudioBuffer<float>& buffer);
    SpectrumAnalyzer* spectrumAnalyzer = nullptr;

    // Curva de equalizacao
    std::vector<float> getEqCurve(int numPoints, float sampleRate); // Calcula a curva
    
    // Sistema de cache para curva de equalização, evitando redesenhos desnecessários
    std::atomic<bool> eqCurveNeedsUpdate { true };
    std::vector<float> cachedEqCurve;

    void updateCachedEqCurve(int numPoints, float sampleRate);

private:
    //====================================Defini��o do filtro==========================================
    std::vector<juce::dsp::ProcessorDuplicator<
    juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Coefficients<float>
>> filters; // Vetor para múltiplos filtro
    juce::dsp::ProcessSpec spec;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamEqAudioProcessor)
    
    // armazena os valores anteriores do filtro para otimização
    std::array<std::atomic<float>, NUM_BANDS> lastFreq;
    std::array<std::atomic<float>, NUM_BANDS> lastQ;
    std::array<std::atomic<float>, NUM_BANDS> lastGain;

    std::array<std::atomic<FilterType>, NUM_BANDS> filterTypes;
    std::array<std::atomic<FilterType>, NUM_BANDS> lastFilterType;

    juce::CriticalSection analyzerLock;

    // Cria layout de parametros
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

};

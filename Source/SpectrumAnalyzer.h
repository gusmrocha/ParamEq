// ParamEQ - Parametric Equalizer Plugin
// Copyright (C) 2025 Gustavo Rocha
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
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// Declaração antecipada do processador de áudio para evitar dependências circulares.
class ParamEqAudioProcessor;

class SpectrumAnalyzer : public juce::Component,
                         public juce::Timer,
                         public juce::AudioProcessorParameter::Listener
{
public:
    explicit SpectrumAnalyzer(ParamEqAudioProcessor&);
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics&) override;
    void timerCallback() override;
    void parameterValueChanged(int, float) override;
    void parameterGestureChanged(int, bool) override {};

    // Métodos para alimentar o analisador
    void pushBuffer(const juce::AudioBuffer<float>& buffer);
    void pushSample(float sample);

private:
    void processFFT();
    void createFrequencyPlotPath(juce::Path& path, const juce::Rectangle<int> bounds);
    void createEQCurvePlot(juce::Graphics& g, const juce::Rectangle<int> bounds);

    // Desenho de grades de referência
    void drawDbGrid(juce::Graphics& g, const juce::Rectangle<int> bounds);
    void drawFrequencyGrid(juce::Graphics& g, const juce::Rectangle<int> bounds);

    // Referência ao processador de áudio
    ParamEqAudioProcessor& processor;

    // Configurações de visualização
    juce::Path spectrumPath;
    juce::CriticalSection pathLock;
    juce::CriticalSection bufferLock;
    
    // Configurações FFT
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT forwardFFT;
    
    // Buffers
    juce::AudioBuffer<float> fftBuffer;
    juce::AudioBuffer<float> fifoBuffer;
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady{false};
};
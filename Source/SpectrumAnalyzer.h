#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// 1. Primeiro faça uma declaração antecipada correta
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


    // Referência ao processador de áudio
    ParamEqAudioProcessor& processor;

    // Configurações de visualização
    juce::Path spectrumPath;
    juce::CriticalSection pathLock;
    juce::CriticalSection bufferLock;
    
    // Configuração FFT
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT forwardFFT;
    
    // Buffers
    juce::AudioBuffer<float> fftBuffer;  // Substitui fftData
    juce::AudioBuffer<float> fifoBuffer; // Substitui fifo
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady{false};
};
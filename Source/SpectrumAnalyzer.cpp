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

#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer(ParamEqAudioProcessor& p) 
    : processor(p),
      forwardFFT(fftOrder),
      fftBuffer(1, fftSize * 2),  // 1 canal, 2*fftSize samples
      fifoBuffer(1, fftSize)      // 1 canal, fftSize samples
{
    setBufferedToImage(true); // Habilita double buffering
    startTimerHz(40); // Atualização a 40 FPS
    setOpaque(true);
    fftBuffer.clear();
    fifoBuffer.clear();
    fftBuffer.clear();
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    stopTimer(); // Para o timer antes de destruir
    juce::ScopedLock sl(bufferLock);
}

// Alimenta o analisador com um buffer já normalizado
void SpectrumAnalyzer::pushBuffer(const juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    const float* channelData = buffer.getReadPointer(0);
    for (int i = 0; i < numSamples; ++i) {
        pushSample(channelData[i]);
    }
}

// Processamento individual de amostras
void SpectrumAnalyzer::pushSample(float sample) {
    jassert(fifoIndex >= 0 && fifoIndex < fftSize);
    // Escreve no buffer FIFO de forma segura
    const juce::ScopedLock sl(bufferLock);
    fifoBuffer.setSample(0, fifoIndex, sample);

    if (++fifoIndex >= fftSize) {
        fifoIndex = 0;
        nextFFTBlockReady = true;
        processFFT();
    }
}

// Calcula a FFT
void SpectrumAnalyzer::processFFT() {
    // Aplica um lock nos ponteiros de escrita e leitura do buffer FIFO
    {
        juce::ScopedLock sl(bufferLock);
        std::memcpy(
            fftBuffer.getWritePointer(0),
            fifoBuffer.getReadPointer(0),
            fftSize * sizeof(float)
        );
    }

    // Aplica janela de Hann (fora do lock para evitar bloqueios)
    juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann);
    window.multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fftSize);

    // Executa FFT
    forwardFFT.performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));

    // Normaliza os valores da FFT
    for (int i = 0; i < fftSize; ++i) {
    fftBuffer.getWritePointer(0)[i] /= fftSize;
    }

    nextFFTBlockReady = true;
}

// Renderização do espectro e curva de equalização
void SpectrumAnalyzer::paint(juce::Graphics& g) {
    // Fundo preto
    g.fillAll(juce::Colours::black);

    // Desenha as grades de referência
    drawDbGrid(g, getLocalBounds());        // horizontais
    drawFrequencyGrid(g, getLocalBounds()); //verticais

    // Obtém e desenha o espectro de áudio (com lock)
    juce::Path local_SpectrumPath;
    {
        juce::ScopedLock sl(bufferLock);
        createFrequencyPlotPath(local_SpectrumPath, getLocalBounds());
    }
    g.setColour(juce::Colours::cyan.withAlpha(0.7f));
    g.fillPath(local_SpectrumPath);

    // Obtém a curva de equalização
   createEQCurvePlot(g, getLocalBounds());

    // Desenha o contorno do espectro
    g.setColour(juce::Colours::white);
    g.strokePath(local_SpectrumPath, juce::PathStrokeType(1.0f));
}

// Cria o caminho para o gráfico de frequência
void SpectrumAnalyzer::createFrequencyPlotPath(juce::Path& path, const juce::Rectangle<int> bounds) {
    path.clear();
    path.startNewSubPath(bounds.getX(), bounds.getBottom());
    
    const float sampleRate = processor.getSampleRate();
    const float xScale = bounds.getWidth() / std::log10(20000.0f / 20.0f);
    // Obtém o ponteiro de leitura do buffer FFT
    const float* fftData = fftBuffer.getReadPointer(0);
    const float minDb = -100.0f;  // Mínimo = -100 dB
    const float maxDb = 6.0f;     // Máximo = +6 dB (permite clipping visual)

    for (int bin = 0; bin < fftSize / 2; ++bin) {
        const float freq = bin * sampleRate / fftSize;
        if (freq < 20.0f || freq > 20000.0f) continue;

        // Converte magnitude para dB e aplica limites
        float magnitudeDb = juce::Decibels::gainToDecibels(
            juce::jmax(fftData[bin], std::pow(10.0f, minDb / 20.0f)) // Evita -Inf
        );
        magnitudeDb = juce::jlimit(minDb, maxDb, magnitudeDb);

        // Mapeia a frequência para a posição X e a magnitude para a posição Y
        float x = bounds.getX() + std::log10(freq / 20.0f) * xScale;
        float y = juce::jmap(magnitudeDb, minDb, maxDb, (float)bounds.getBottom(), (float)bounds.getY());

        // Garante que o Path não saia dos limites
        if (y < static_cast<float>(bounds.getY())) y = static_cast<float>(bounds.getY());
        if (y > static_cast<float>(bounds.getBottom())) y = static_cast<float>(bounds.getBottom());

        path.lineTo(x, y);
    }

    // Feche o Path corretamente (opcional, para preenchimento)
    path.lineTo(static_cast<float>(bounds.getRight()), static_cast<float>(bounds.getBottom()));
}

// Cria o gráfico da curva de equalização
void SpectrumAnalyzer::createEQCurvePlot(juce::Graphics& g, const juce::Rectangle<int> bounds) {
    const float width = static_cast<float>(bounds.getWidth());
    const float height = static_cast<float>(bounds.getHeight());

    // Constantes de escala
    constexpr float minDb = -24.0f;
    constexpr float maxDb = 24.0f;
    
    // Função de mapeamento com verificação de limites
    auto dbToY = [height, minDb, maxDb](float db, bool& isValid) {
        isValid = (db >= minDb && db <= maxDb);
        return juce::jmap(db, minDb, maxDb, height, 0.0f);
    };
    
    // Obtém os dados da curva do Cache
    const auto& eqCurve = processor.cachedEqCurve;
    
    if (eqCurve.empty() || eqCurve.size() < bounds.getWidth())
    return;
    
    // Cria o path da curva
    juce::Path eqPath;
    bool isFirstPoint = true;
    bool currentValid = false;
    bool previousValid = false;
    
    for (int x = 0; x < bounds.getWidth(); ++x) {
        float magnitudeDb = eqCurve[x];
        float y = dbToY(magnitudeDb, currentValid);
        
        if (currentValid) {
            if (!previousValid) {
                // Começa um novo subpath quando a curva entra na área visível
                eqPath.startNewSubPath(static_cast<float>(x), y);
                isFirstPoint = false;
            } else {
                // Continua a linha se estiver dentro dos limites
                eqPath.lineTo(static_cast<float>(x), y);
            }
        } else if (previousValid) {
            // Finaliza o subpath quando a curva sai da área visível
            eqPath.lineTo(static_cast<float>(x), y);
            eqPath.startNewSubPath(static_cast<float>(x), y);
        }
        
        previousValid = currentValid;
    }    
    // Desenha a curva
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.strokePath(eqPath, juce::PathStrokeType(2.0f));
}

// Callback do timer de atualização
void SpectrumAnalyzer::timerCallback()
{
    if (processor.eqCurveNeedsUpdate)
    {
        processor.updateCachedEqCurve(getWidth(), static_cast<float>(processor.getSampleRate()));
    }

    if (nextFFTBlockReady.exchange(false))
    {
        juce::MessageManager::callAsync([this]() {
            repaint();
        });
    }
}


// Callback quando um parâmetro é alterado
void SpectrumAnalyzer::parameterValueChanged(int, float) {
    repaint();
}

// Desenha as grades de referencia horizontais
void SpectrumAnalyzer::drawDbGrid(juce::Graphics& g, const juce::Rectangle<int> bounds) {
    const float height = static_cast<float>(bounds.getHeight());
    
    // Configurações de estilo
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    
    // Linhas de -21dB a +21dB em passos de 3dB (excluindo -24 e +24)
    for (float db = -21.0f; db <= 21.0f; db += 3.0f) {
        float y = juce::jmap(db, -24.0f, 24.0f, height, 0.0f);
        
        // Linha mais forte para múltiplos de 6dB
        float alpha = (static_cast<int>(db) % 6 == 0) ? 0.5f : 0.2f;
        g.setColour(juce::Colours::grey.withAlpha(alpha));
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
        
        // Rótulos apenas para múltiplos de 12dB (excluindo bordas)
        if (static_cast<int>(db) % 12 == 0 && db != -24.0f && db != 24.0f) {
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            g.drawText(juce::String(static_cast<int>(db)) + " dB", 
                      bounds.getX() + 5, y - 10, 50, 20, 
                      juce::Justification::left);
        }
    }
    
    // Linha de 0dB mais destacada
    float zeroY = juce::jmap(0.0f, -24.0f, 24.0f, height, 0.0f);
    g.setColour(juce::Colours::grey.withAlpha(0.7f));
    g.drawHorizontalLine(static_cast<int>(zeroY), bounds.getX(), bounds.getRight());
}

// Desenha as grades de referencia verticais
void SpectrumAnalyzer::drawFrequencyGrid(juce::Graphics& g, const juce::Rectangle<int> bounds) {
    const float width = static_cast<float>(bounds.getWidth());
    const float height = static_cast<float>(bounds.getHeight());
    const float xScale = width / std::log10(20000.0f / 20.0f);

    const float lineThickness = 0.5f;
    const float mainAlpha = 0.35f;
    const float auxAlpha = 0.15f;

    std::vector<float> freqs;

    // Gera frequências logarítmicas: 20, 30, 40, ..., 90, 100, 200, ..., 10000, ..., 20000
    for (int decade = 1; decade <= 4; ++decade) { // 10^1 até 10^4 (10Hz a 10000Hz)
        float base = std::pow(10.0f, (float)decade);
        for (int i = 1; i < 10; ++i) {
            float f = i * base;
            if (f >= 20.0f && f <= 20000.0f)
                freqs.push_back(f);
        }
    }

    // Adiciona 20000 Hz manualmente
    freqs.push_back(20000.0f);

    // Desenha as linhas
    for (float freq : freqs) {
        float x = std::log10(freq / 20.0f) * xScale + bounds.getX();

        bool isMain = (freq == 100.0f || freq == 1000.0f || freq == 10000.0f);

        g.setColour(juce::Colours::grey.withAlpha(isMain ? mainAlpha : auxAlpha));
        g.drawVerticalLine((int)x, (float)bounds.getY(), (float)bounds.getBottom());

        if (isMain) {
            juce::String label = (freq < 1000.0f)
                ? juce::String(freq, 0) + " Hz"
                : juce::String(freq / 1000.0f, 1) + " kHz";

            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.drawText(label, x - 25, bounds.getBottom() - 20, 60, 20, juce::Justification::centred);
        }
    }
}

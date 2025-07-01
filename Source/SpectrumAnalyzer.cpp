#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer(ParamEqAudioProcessor& p) 
    : processor(p),
      forwardFFT(fftOrder),
      fftBuffer(1, fftSize * 2),  // 1 canal, 2*fftSize samples
      fifoBuffer(1, fftSize)      // 1 canal, fftSize samples
{
    setBufferedToImage(true); // Habilita double buffering
    startTimerHz(30); // Atualização a 30 FPS
    setOpaque(true);
    fftBuffer.clear();
    fifoBuffer.clear();
    fftBuffer.clear();
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    //processor.parameters.removeParameterListener("FREQ", this);
    stopTimer(); // **Para o timer antes de destruir**
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
    // **Use um escopo menor para o lock** (só durante a cópia)
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

void SpectrumAnalyzer::createEQCurvePlot(juce::Graphics& g, const juce::Rectangle<int> bounds) {
    const float width = static_cast<float>(bounds.getWidth());
    const float height = static_cast<float>(bounds.getHeight());

    // 1. Constantes de escala (ajuste conforme necessário)
    constexpr float minDb = -15.0f;
    constexpr float maxDb = 15.0f;
    
    // 2. Função de mapeamento corrigida
    auto dbToY = [height, minDb, maxDb](float db) {
        return juce::jmap(db, minDb, maxDb, height, 0.0f);
    };
    
    // 3. Obtém os dados da curva (já em dB)
    auto eqCurve = processor.getEqCurve(bounds.getWidth(), 
                                      static_cast<float>(processor.getSampleRate()));
    
    // 4. Cria o path da curva
    juce::Path eqPath;
    bool isFirstPoint = true;
    
    for (int x = 0; x < bounds.getWidth(); ++x) {
        float magnitudeDb = eqCurve[x]; // Já está em dB
        magnitudeDb = juce::jlimit(minDb, maxDb, magnitudeDb);
        float y = dbToY(magnitudeDb);
        
        if (isFirstPoint) {
            eqPath.startNewSubPath(0.0f, y);
            isFirstPoint = false;
        } else {
            eqPath.lineTo(static_cast<float>(x), y);
        }
    }
    
    // 5. Desenha a linha de referência
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    float zeroDbY = dbToY(0.0f);
    g.drawHorizontalLine(static_cast<int>(zeroDbY), 
                        bounds.getX(), bounds.getRight());
    
    // 6. Desenha a curva
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.strokePath(eqPath, juce::PathStrokeType(2.0f));
}

void SpectrumAnalyzer::timerCallback() {
    if (nextFFTBlockReady.exchange(false)) {
        // Garante que o repaint() seja executado na thread da GUI
        juce::MessageManager::callAsync([this]() {
            repaint();
        });
    }
}

void SpectrumAnalyzer::parameterValueChanged(int, float) {
    repaint();
}
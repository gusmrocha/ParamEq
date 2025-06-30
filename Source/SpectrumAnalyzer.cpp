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

// Renderização do espectro
void SpectrumAnalyzer::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey.withAlpha(0.5f));

    // Obtém a curva do EQ do processador
    auto eqCurve = processor.getEqCurve(getWidth(), processor.getSampleRate());

    // Desenha a curva do EQ
    juce::Path eqPath;
    eqPath.startNewSubPath(0, getHeight());

    juce::Path path;
    {
        juce::ScopedLock sl(bufferLock); // **Trava durante a criação do Path**
        createFrequencyPlotPath(path, getLocalBounds());
    }

    // Desenha a grade de frequência
    for (float freq : {20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f}) {
        float x = std::log10(freq / 20.0f) * getWidth() / std::log10(20000.0f / 20.0f);
        g.drawVerticalLine(x, 0, getHeight());
    }
    
    {
        juce::ScopedLock lock(pathLock);
        g.setColour(juce::Colours::cyan.withAlpha(0.7f));
        g.fillPath(path);
    }
    // Grade logarítmica
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    for (float freq : {20.0f, 100.0f, 1000.0f, 10000.0f}) {
        float x = std::log10(freq / 20.0f) * getWidth() / std::log10(20000.0f / 20.0f);
        g.drawVerticalLine(x, 0, getHeight());
    }
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

        // **Adicione um limite mínimo para evitar valores muito baixos**
         // Converte magnitude para dB e aplica limites
        float magnitudeDb = juce::Decibels::gainToDecibels(
            juce::jmax(fftData[bin], std::pow(10.0f, minDb / 20.0f)) // Evita -Inf
        );
        magnitudeDb = juce::jlimit(minDb, maxDb, magnitudeDb);
        
        // const float magnitude = juce::Decibels::gainToDecibels(fftData[bin]);
        float x = bounds.getX() + std::log10(freq / 20.0f) * xScale;
        float y = juce::jmap(magnitudeDb, minDb, maxDb, (float)bounds.getBottom(), (float)bounds.getY());

        // **Garanta que o Path não saia dos limites**
        if (y < static_cast<float>(bounds.getY())) y = static_cast<float>(bounds.getY());
        if (y > static_cast<float>(bounds.getBottom())) y = static_cast<float>(bounds.getBottom());

        path.lineTo(x, y);
    }

    // **Feche o Path corretamente (opcional, para preenchimento)**
    path.lineTo(static_cast<float>(bounds.getRight()), static_cast<float>(bounds.getBottom()));
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
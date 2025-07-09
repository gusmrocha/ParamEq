#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SpectrumAnalyzer.h"

juce::String ParamEqAudioProcessor::getFilterTypeName(FilterType type) {
    switch (type) {
        case PEAK:      return "Peak";
        case LOW_SHELF: return "Low Shelf";
        case HIGH_SHELF: return "High Shelf";
        case LOW_PASS:   return "Low Pass";
        case HIGH_PASS:  return "High Pass";
        default:        return "Unknown";
    }
}

//=================================== Construtor/Destrutor ===========================================
ParamEqAudioProcessor::ParamEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this, nullptr, "Params", createParameterLayout())
{
    // Inicialize os filtros (mantenha o existente)
    filters.resize(NUM_BANDS);
    
    // Inicialize valores anteriores para otimização
    for (auto& val : lastFreq) val.store(-1.0f);
    for (auto& val : lastQ) val.store(-1.0f);
    for (auto& val : lastGain) val.store(-1.0f);
    for (auto& type : filterTypes) type = PEAK;
    for (auto& lastType : lastFilterType) lastType = PEAK;
}

juce::AudioProcessorValueTreeState::ParameterLayout ParamEqAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int band = 0; band < NUM_BANDS; ++band)
    {
        // Parâmetro de frequência
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "FREQ" + juce::String(band + 1),
            "Frequency " + juce::String(band + 1),
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
            1000.0f
        ));

        // Parâmetro de ganho
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "GAIN" + juce::String(band + 1),
            "Gain " + juce::String(band + 1),
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
            0.0f
        ));

        // Parâmetro de Q
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "Q" + juce::String(band + 1),
            "Q " + juce::String(band + 1),
            juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
            1.0f
        ));

        // Parâmetro de tipo de filtro
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "TYPE" + juce::String(band + 1),
            "Filter Type " + juce::String(band + 1),
            juce::StringArray({"Peak", "Low Shelf", "High Shelf", "Low Pass", "High Pass"}),
            0 // Valor padrão: Peak
        ));
    }

    return {params.begin(), params.end()};
}

ParamEqAudioProcessor::~ParamEqAudioProcessor() //Destrutor da classe
{
    const juce::ScopedLock sl(analyzerLock);
    spectrumAnalyzer = nullptr;
}

//================================= Inicializações midi, nome e presets ====================================
const juce::String ParamEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParamEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParamEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParamEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParamEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParamEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParamEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParamEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParamEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParamEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//================================== buffer =========================================
void ParamEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Configura o ProcessSpec com informações do host
    spec.sampleRate = sampleRate;  // Taxa de amostragem do projeto
    spec.maximumBlockSize = samplesPerBlock;  // Tamanho máximo do buffer
    spec.numChannels = getTotalNumOutputChannels();  // Número de canais

    // Prepara o filtro com as especificações
    for(auto& filter : filters) { // Prepara todos os filtros
        filter.prepare(spec);
    }

}



void ParamEqAudioProcessor::releaseResources()
{
    juce::ScopedLock sl(analyzerLock);
    spectrumAnalyzer = nullptr; // **Seguro se a GUI for fechada**
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParamEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

// Envia o buffer para o analisador de espectro
void ParamEqAudioProcessor::pushBufferToAnalyzer(const juce::AudioBuffer<float>& buffer)
{
    const juce::ScopedLock sl(analyzerLock);
    if (spectrumAnalyzer != nullptr && buffer.getNumSamples() > 0)
        spectrumAnalyzer->pushBuffer(buffer);
}

FilterType getMappedFilterType(int choiceIndex)
{
    switch (choiceIndex) {
        case 0: return PEAK;
        case 1: return LOW_SHELF;
        case 2: return HIGH_SHELF;
        case 3: return LOW_PASS;
        case 4: return HIGH_PASS;
        default: return PEAK;
    }
}


std::vector<float> ParamEqAudioProcessor::getEqCurve(int numPoints, float sampleRate) 
{
    std::vector<float> curve(numPoints, 0.0f);

    for (int i = 0; i < numPoints; ++i) 
    {
        float freq = juce::mapToLog10(float(i) / (numPoints - 1), 20.0f, 20000.0f);
        float totalMagnitude = 1.0f;
        
        for (int band = 0; band < NUM_BANDS; ++band) 
        {
            auto freqParam = parameters.getRawParameterValue("FREQ" + juce::String(band + 1))->load();
            auto qParam = parameters.getRawParameterValue("Q" + juce::String(band + 1))->load();
            auto gainDb = parameters.getRawParameterValue("GAIN" + juce::String(band + 1))->load();
            auto typeValue = parameters.getRawParameterValue("TYPE" + juce::String(band + 1))->load();
            FilterType currentType = getMappedFilterType(static_cast<int>(typeValue));

            if (currentType == LOW_PASS || currentType == HIGH_PASS || std::abs(gainDb) > 0.1f) 
            {
                // Usamos ponteiro bruto temporário para os coeficientes
                juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>> coeffs;
                
                switch (currentType) {
                    case PEAK:
                        coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                            sampleRate, freqParam, qParam, juce::Decibels::decibelsToGain(gainDb));
                        break;
                    case LOW_SHELF:
                        coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                            sampleRate, freqParam, qParam, juce::Decibels::decibelsToGain(gainDb));
                        break;
                    case HIGH_SHELF:
                        coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                            sampleRate, freqParam, qParam, juce::Decibels::decibelsToGain(gainDb));
                        break;
                    case LOW_PASS:
                        coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
                            sampleRate, freqParam, qParam);
                        break;
                    case HIGH_PASS:
                        coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
                            sampleRate, freqParam, qParam);
                        break;
                }
                
                if (coeffs != nullptr) {
                    totalMagnitude *= coeffs->getMagnitudeForFrequency(freq, sampleRate);
                }
            }
        }
        
        curve[i] = juce::Decibels::gainToDecibels(totalMagnitude);
    }

    return curve;
}

void ParamEqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) 
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    // 1. Configuração inicial
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // 2. Processamento principal (série com otimizações)
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    for (int band = 0; band < NUM_BANDS; ++band) 
    {
        auto freq = parameters.getRawParameterValue("FREQ" + juce::String(band + 1))->load();
        auto q = parameters.getRawParameterValue("Q" + juce::String(band + 1))->load();
        auto gainDb = parameters.getRawParameterValue("GAIN" + juce::String(band + 1))->load();
        FilterType currentType = getMappedFilterType(
            parameters.getRawParameterValue("TYPE" + juce::String(band + 1))->load()
        );

        // Pule filtros inativos (exceto passa-baixa/alta)
        if (std::abs(gainDb) < 0.1f && currentType != LOW_PASS && currentType != HIGH_PASS) continue;

        // Atualize coeficientes se necessário
        if (freq != lastFreq[band] || q != lastQ[band] || gainDb != lastGain[band] || 
            (band == 0 || band == 7) && currentType != lastFilterType[band]) 
        {
            switch (currentType) {
                case PEAK:
                    *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                        spec.sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));
                    break;
                    
                case LOW_SHELF:
                    *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                        spec.sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));
                    break;
                    
                case HIGH_SHELF:
                    *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                        spec.sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));
                    break;
                    
                case LOW_PASS:
                    *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
                        spec.sampleRate, freq, q);
                    break;
                    
                case HIGH_PASS:
                    *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                        spec.sampleRate, freq, q);
                    break;
            }
            
            // Armazena últimos valores
            lastFreq[band] = freq;
            lastQ[band] = q;
            lastGain[band] = gainDb;
            if (band == 0 || band == 7) {
                lastFilterType[band] = currentType;
            }
        }

        filters[band].process(context);
    }
    // Análise de espectro
    if (spectrumAnalyzer != nullptr) 
    {
        juce::AudioBuffer<float> monoBuffer(1, numSamples);
        const float gainFactor = 1.0f / std::sqrt(numChannels);
        
        for (int i = 0; i < numSamples; ++i) 
        {
            float sum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) 
            {
                sum += buffer.getSample(ch, i);
            }
            monoBuffer.setSample(0, i, gainFactor * sum);
        }
        pushBufferToAnalyzer(monoBuffer);
    }
}

//==============================================================================
bool ParamEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParamEqAudioProcessor::createEditor()
{
    return new ParamEqAudioProcessorEditor (*this);
}

//================================ State do plugin =================================
void ParamEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParamEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParamEqAudioProcessor();
}

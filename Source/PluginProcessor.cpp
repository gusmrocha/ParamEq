/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//=================================== Construtor/Destrutor ===========================================
ParamEqAudioProcessor::ParamEqAudioProcessor() // Construtor da classe
#ifndef JucePlugin_PreferredChannelConfigurations  
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect  
#if ! JucePlugin_IsSynth  
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif  
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif  
    )
#endif  
    , parameters(*this, nullptr, "Params", {
    std::make_unique<juce::AudioParameterFloat>(
        "FREQ1",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN1",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q1",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ2",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN2",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q2",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ3",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN3",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q3",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ4",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN4",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q4",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ5",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN5",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q5",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ6",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN6",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q6",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ7",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN7",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q7",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    ),
        std::make_unique<juce::AudioParameterFloat>(
        "FREQ8",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN8",
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q8",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 7.0f, 0.01f),
        1.0f
    )
        })
    {
        filters.resize(NUM_BANDS); // Inicializa o vetor de filtros
    }   

ParamEqAudioProcessor::~ParamEqAudioProcessor() //Destrutor da classe
{
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
	// Aqui você pode liberar recursos, se necessário.
	// O JUCE geralmente cuida disso automaticamente.
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

void ParamEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Limpeza do buffer por causa de erros etc
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

        for(int band = 0; band < NUM_BANDS; band++) {
            for(int channel = 0; channel < totalNumInputChannels; ++channel) {
            // Obter parâmetros para cada banda
                auto freqParam = parameters.getRawParameterValue("FREQ" + juce::String(band+1));
                auto qParam = parameters.getRawParameterValue("Q" + juce::String(band+1));
                auto gainParam = parameters.getRawParameterValue("GAIN" + juce::String(band+1));
                
                float freq = freqParam->load();
                float q = qParam->load();
                float gain = juce::Decibels::decibelsToGain(gainParam->load());
            // Configurar filtro
                *filters[band].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                spec.sampleRate, freq, q, gain
            );
            }
        
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        filters[band].process(context); // Processar cada filtro
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

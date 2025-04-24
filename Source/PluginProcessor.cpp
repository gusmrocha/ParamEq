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
        "FREQ",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "GAIN",
        "Gain",
        juce::NormalisableRange<float>(-16.0f, 16.0f, 0.1f),
        0.0f
    ),
    std::make_unique<juce::AudioParameterFloat>(
        "Q",
        "Q Factor",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f),
        1.0f
    )
        })
    , spec{} // Inicializa a classe com os valores padrão  
{
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
    filter.prepare(spec);

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

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        //conversão do valor do ganho em dB para linear
		const float gain_lin = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("GAIN")->load());

        // Gera o filtro a partir dos parametros fornecidos de freq, q e ganho
        *filter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            spec.sampleRate,
            *parameters.getRawParameterValue("FREQ"),
            *parameters.getRawParameterValue("Q"),
            gain_lin
        );

        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        filter.process(context);

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

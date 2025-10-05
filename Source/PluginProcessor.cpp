/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LorenzAudioProcessor::LorenzAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif // JucePlugin_PreferredChannelConfigurations
{
    alphaParam = apvts.getRawParameterValue("ALPHA");
    betaDuffingParam = apvts.getRawParameterValue("BETA_DUFFING");
    gammaParam = apvts.getRawParameterValue("GAMMA");
    deltaParam = apvts.getRawParameterValue("DELTA");
    frequencyParam = apvts.getRawParameterValue("FREQUENCY");
    timestepRangedParam = static_cast<juce::RangedAudioParameter*>(apvts.getParameter("TIMESTEP"));
    timestepParam = apvts.getRawParameterValue("TIMESTEP");
    outputLevelParam = apvts.getRawParameterValue("OUTPUT_LEVEL");
    viewZoomXParam = apvts.getRawParameterValue("VIEW_ZOOM_X");
    viewZoomZParam = apvts.getRawParameterValue("VIEW_ZOOM_Z");
}


LorenzAudioProcessor::~LorenzAudioProcessor()
{
}

//==============================================================================
const juce::String LorenzAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LorenzAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LorenzAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LorenzAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LorenzAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LorenzAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LorenzAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LorenzAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LorenzAudioProcessor::getProgramName (int index)
{
    return {};
}

void LorenzAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LorenzAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lorenzOsc.prepareToPlay(sampleRate);

    lorenzOsc.setParameters(alphaParam, betaDuffingParam, gammaParam, deltaParam, frequencyParam);
    lorenzOsc.setTimestep(timestepParam);

    // Calculate how many audio samples to wait before generating the next point for the GUI
    pointGenerationInterval = static_cast<int>(sampleRate / pointsPerSecond);
}

void LorenzAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LorenzAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LorenzAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin.
    // As our oscillator is generating the signal, we can clear the buffer first.

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get pointers to the left and right channels.
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // --- Handle reset request ---
    if (resetRequested.exchange(false))
    {
        lorenzOsc.reset();
    }

    // Load parameter values.
    const float outputLevel = juce::Decibels::decibelsToGain(outputLevelParam->load());

    // The state variables can have a large range, so we scale them down.
    // These are empirical values, you may want to adjust them.
    const float xScale = 1.0f; // Duffing output is often already in a good range

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto [x, y, z] = lorenzOsc.getNextSample();

        // Push points to the FIFO at a controlled rate, not on every sample.
        if (--samplesUntilNextPoint <= 0)
        {
            pushPointToFifo({(float)x, (float)y, (float)z});
            samplesUntilNextPoint = pointGenerationInterval;
        }

        // Scale and apply gain to each component
        const float xSample = static_cast<float>(x) * xScale;

        // Mix all sources and apply master output level
        leftChannel[sample]  = xSample * outputLevel;
        rightChannel[sample] = xSample * outputLevel;
    }
    
    // In case of more than 2 output channels, copy the stereo signal to them.
    for (int channel = 2; channel < totalNumOutputChannels; ++channel)
        buffer.copyFrom(channel, 0, buffer, channel % 2, 0, buffer.getNumSamples());

}

//==============================================================================
bool LorenzAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LorenzAudioProcessor::createEditor()
{
    return new LorenzAudioProcessorEditor (*this);
}

void LorenzAudioProcessor::requestOscillatorReset()
{
    resetRequested = true;
}

//==============================================================================
void LorenzAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LorenzAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LorenzAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout LorenzAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Duffing Equation Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("ALPHA", "Alpha",
                                                           juce::NormalisableRange<float>(-2.0f, 2.0f, 0.01f), -1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("BETA_DUFFING", "Beta",
                                                           juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("GAMMA", "Gamma",
                                                           juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("DELTA", "Delta",
                                                           juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("FREQUENCY", "Frequency",
                                                           juce::NormalisableRange<float>(20.f, 2000.0f, 1.0f, 0.25f), 440.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("TIMESTEP", "Timestep",
                                                           juce::NormalisableRange<float>(0.0001f, 0.02f, 0.0001f, 0.5f),
                                                           0.01f));

    // --- Output Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("OUTPUT_LEVEL", "Output Level",
                                                           juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));

    // --- View Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("VIEW_ZOOM_X", "View Zoom X",
                                                           juce::NormalisableRange<float>(10.0f, 100.0f, 0.1f, 0.5f),
                                                           50.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("VIEW_ZOOM_Z", "View Zoom Z",
                                                           juce::NormalisableRange<float>(10.0f, 100.0f, 0.1f, 0.5f),
                                                           50.0f));

    return layout;
}
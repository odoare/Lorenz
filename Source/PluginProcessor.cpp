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
      ,
      sigmaParam(apvts.getRawParameterValue("SIGMA")),
      rhoParam(apvts.getRawParameterValue("RHO")),
      betaParam(apvts.getRawParameterValue("BETA")),
      timestepParam(apvts.getRawParameterValue("TIMESTEP")),
      levelXParam(apvts.getRawParameterValue("LEVEL_X")),
      panXParam(apvts.getRawParameterValue("PAN_X")),
      levelYParam(apvts.getRawParameterValue("LEVEL_Y")),
      panYParam(apvts.getRawParameterValue("PAN_Y")),
      levelZParam(apvts.getRawParameterValue("LEVEL_Z")),
      panZParam(apvts.getRawParameterValue("PAN_Z")),
      outputLevelParam(apvts.getRawParameterValue("OUTPUT_LEVEL")),
      viewZoomXParam(apvts.getRawParameterValue("VIEW_ZOOM_X")),
      viewZoomZParam(apvts.getRawParameterValue("VIEW_ZOOM_Z"))
#endif // JucePlugin_PreferredChannelConfigurations
{
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

    // Pass the parameter pointers to the oscillator. This only needs to be done once.
    lorenzOsc.setParameters(sigmaParam, rhoParam, betaParam);
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

    // Load parameter values.
    const float levelX = juce::Decibels::decibelsToGain(levelXParam->load());
    const float panX = panXParam->load();
    const float levelY = juce::Decibels::decibelsToGain(levelYParam->load());
    const float panY = panYParam->load();
    const float levelZ = juce::Decibels::decibelsToGain(levelZParam->load());
    const float panZ = panZParam->load();
    const float outputLevel = juce::Decibels::decibelsToGain(outputLevelParam->load());

    // The state variables can have a large range, so we scale them down.
    // These are empirical values, you may want to adjust them.
    const float xScale = 0.05f;
    const float yScale = 0.05f;
    const float zScale = 0.025f;

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
        const float xSample = static_cast<float>(x) * xScale * levelX;
        const float ySample = static_cast<float>(y) * yScale * levelY;
        const float zSample = static_cast<float>(z) * zScale * levelZ;

        // Apply panning (constant power)
        const float xL = xSample * std::cos((panX + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float xR = xSample * std::sin((panX + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        const float yL = ySample * std::cos((panY + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float yR = ySample * std::sin((panY + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        const float zL = zSample * std::cos((panZ + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float zR = zSample * std::sin((panZ + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        // Mix all sources and apply master output level
        leftChannel[sample] = (xL + yL + zL) * outputLevel;
        rightChannel[sample] = (xR + yR + zR) * outputLevel;
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

    // --- Attractor Parameters ---
    // Classic chaotic values: sigma = 10, rho = 28, beta = 8/3
    layout.add(std::make_unique<juce::AudioParameterFloat>("SIGMA", "Sigma",
                                                           juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f),
                                                           10.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("RHO", "Rho",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
                                                           28.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("BETA", "Beta",
                                                           juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
                                                           8.0f / 3.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("TIMESTEP", "Timestep",
                                                           juce::NormalisableRange<float>(0.0001f, 0.01f, 0.0001f, 0.5f),
                                                           0.001f));
    
    // --- Mixer Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_X", "Level X",
                                                           juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PAN_X", "Pan X",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_Y", "Level Y",
                                                           juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PAN_Y", "Pan Y",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_Z", "Level Z",
                                                           juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PAN_Z", "Pan Z",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

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
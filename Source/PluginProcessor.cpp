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
      timestepRangedParam(static_cast<juce::RangedAudioParameter*>(apvts.getParameter("TIMESTEP"))),
      timestepParam(apvts.getRawParameterValue("TIMESTEP")),
      levelXParam(apvts.getRawParameterValue("LEVEL_X")),
      panXParam(apvts.getRawParameterValue("PAN_X")),
      levelYParam(apvts.getRawParameterValue("LEVEL_Y")),
      panYParam(apvts.getRawParameterValue("PAN_Y")),
      levelZParam(apvts.getRawParameterValue("LEVEL_Z")),
      panZParam(apvts.getRawParameterValue("PAN_Z")),
      outputLevelParam(apvts.getRawParameterValue("OUTPUT_LEVEL")),
      viewZoomXParam(apvts.getRawParameterValue("VIEW_ZOOM_X")),
      viewZoomZParam(apvts.getRawParameterValue("VIEW_ZOOM_Z")),
      viewZoomYParam(apvts.getRawParameterValue("VIEW_ZOOM_Y")),
      attackParam(apvts.getRawParameterValue("ATTACK")),
      decayParam(apvts.getRawParameterValue("DECAY")),
      sustainParam(apvts.getRawParameterValue("SUSTAIN")),
      releaseParam(apvts.getRawParameterValue("RELEASE")),
      modTargetParam(apvts.getRawParameterValue("MOD_TARGET")),
      modAmountParam(apvts.getRawParameterValue("MOD_AMOUNT")),
      targetFrequencyRangedParam(static_cast<juce::RangedAudioParameter*>(apvts.getParameter("TARGET_FREQ"))),
      targetFrequencyParam(apvts.getRawParameterValue("TARGET_FREQ")),
      kpParam(apvts.getRawParameterValue("KP")),
      kiParam(apvts.getRawParameterValue("KI")),
      kdParam(apvts.getRawParameterValue("KD")),
      pitchSourceParam(apvts.getRawParameterValue("PITCH_SOURCE")),
      pitchDetector(44100.0, PITCHBUFFERSIZE), // Initialize with a default sample rate
      mxParam(apvts.getRawParameterValue("MX")), //
      myParam(apvts.getRawParameterValue("MY")),
      mzParam(apvts.getRawParameterValue("MZ")),
      cxParam(apvts.getRawParameterValue("CX")),
      cyParam(apvts.getRawParameterValue("CY")),
      czParam(apvts.getRawParameterValue("CZ"))
      , tamingParam(apvts.getRawParameterValue("TAMING"))
#endif
{
    dtTarget = timestepParam->load();
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
   return true;
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

    // Pass the parameter pointers to the oscillator.
    lorenzOsc.setParameters(sigmaParam, rhoParam, betaParam, mxParam, myParam, mzParam, cxParam, cyParam, czParam, tamingParam);
    lorenzOsc.setTimestep(timestepParam);

    // Prepare ADSR
    ampAdsr.setSampleRate(sampleRate);

    // Calculate how many audio samples to wait before generating the next point for the GUI
    pointGenerationInterval = static_cast<int>(sampleRate / pointsPerSecond);

    // Prepare frequency detector
    nPitchBuffers = PITCHBUFFERSIZE/samplesPerBlock;
    pitchBufferSize = nPitchBuffers*samplesPerBlock;
    bufferSize = samplesPerBlock;

    pitchDetector.setBufferSize (pitchBufferSize);
    pitchDetector.setSampleRate (sampleRate);
    analysisBuffer.setSize(1, pitchBufferSize);

    // Initialize HPF state arrays to match the number of output channels
    hpf_prevInput.resize(getTotalNumOutputChannels());
    hpf_prevOutput.resize(getTotalNumOutputChannels());
    processSampleRate = sampleRate;
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

void LorenzAudioProcessor::highPassFilter(juce::AudioBuffer<float>& buffer, float cutoffFreq)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Ensure state arrays are the correct size
    jassert(hpf_prevInput.size() == numChannels);
    jassert(hpf_prevOutput.size() == numChannels);

    // Filter coefficients (RC filter)
    float RC = 1.0f / (juce::MathConstants<float>::twoPi * cutoffFreq);
    float dt = 1.0f / processSampleRate;
    float alpha = RC / (RC + dt);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        float prevInput = hpf_prevInput[channel];
        float prevOutput = hpf_prevOutput[channel];

        for (int n = 0; n < numSamples; ++n)
        {
            float input = channelData[n];
            channelData[n] = alpha * (prevOutput + input - prevInput);
            prevOutput = channelData[n];
            prevInput = input;
        }
        hpf_prevInput.set(channel, prevInput);
        hpf_prevOutput.set(channel, prevOutput);
    }
}

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

    // --- Update ADSR Parameters ---
    ampAdsrParams.attack = attackParam->load();
    ampAdsrParams.decay = decayParam->load();
    ampAdsrParams.sustain = sustainParam->load();
    ampAdsrParams.release = releaseParam->load();
    ampAdsr.setParameters(ampAdsrParams);

    // --- Modulation Setup ---
    const auto modAmount = modAmountParam->load();
    const auto modTarget = static_cast<int>(modTargetParam->load());
    // Create a map for easy lookup of parameters by their index in the choice parameter
    const std::map<int, std::atomic<float>*> modTargetMap = { {1, sigmaParam}, {2, rhoParam}, {3, betaParam}, {4, mxParam}, {5, myParam}, {6, mzParam}, {7, cxParam}, {8, cyParam}, {9, czParam}, {10, tamingParam} };
    // Create a map to get the parameter ID string from the choice index
    const std::map<int, juce::String> modTargetIdMap = { {1, "SIGMA"}, {2, "RHO"}, {3, "BETA"}, {4, "MX"}, {5, "MY"}, {6, "MZ"}, {7, "CX"}, {8, "CY"}, {9, "CZ"}, {10, "TAMING"} };


    // --- MIDI Event Handling ---
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            const int noteNumber = msg.getNoteNumber();
            
            // Add note to the stack if it's not already there
            if (!noteStack.contains(noteNumber))
                noteStack.add(noteNumber);

            if (currentNote != noteNumber)
            {
                currentNote = noteNumber;
                // If this is the first note being played, trigger the attack.
                // Otherwise, we just change frequency (legato style).
                if (noteStack.size() == 1)
                    ampAdsr.noteOn();
            }
            
            // Update target frequency based on the new note
            const float newFreq = (float) juce::MidiMessage::getMidiNoteInHertz(currentNote);
            // Convert the frequency to a normalized value (0.0 to 1.0) and set the parameter.
            // This ensures the GUI is notified of the change.
            auto normalizedFreq = targetFrequencyRangedParam->getNormalisableRange().convertTo0to1(newFreq);
            targetFrequencyRangedParam->setValueNotifyingHost(normalizedFreq);
        }
        else if (msg.isNoteOff())
        {
            const int noteNumber = msg.getNoteNumber();
            noteStack.removeFirstMatchingValue(noteNumber);

            // If the released note was the one playing, trigger release.
            if (currentNote == noteNumber)
            {
                // If other notes are still held, switch to the last one on the stack.
                if (noteStack.size() > 0)
                {
                    currentNote = noteStack.getLast();
                    const float newFreq = (float) juce::MidiMessage::getMidiNoteInHertz(currentNote);
                    auto normalizedFreq = targetFrequencyRangedParam->getNormalisableRange().convertTo0to1(newFreq);
                    targetFrequencyRangedParam->setValueNotifyingHost(normalizedFreq);
                }
                else // Otherwise, trigger release and reset note state.
                {
                    ampAdsr.noteOff();
                    currentNote = -1;
                }
            }
        }
        else if (msg.isController() && msg.getControllerNumber() == 1)
        {
            // Store the last CC01 value, normalized to 0.0 - 1.0
            lastCC01Value = msg.getControllerValue() / 127.0f;
            //std::cout << "CC01: " << lastCC01Value << std::endl;
        }
    }
    midiMessages.clear(); // We've processed the MIDI messages

    buffer.clear();

    // Create a temporary buffer to hold the signal for pitch analysis for the current block.
    juce::AudioBuffer<float> pitchAnalysisBlock(1, buffer.getNumSamples());

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get pointers to the left and right channels.
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // --- Handle reset request ---
    if (resetRequested.exchange(false))
    {
        lorenzOsc.reset();
        // Also reset the PI controller state
        dtIntegral = 0.0f;
        dtProportional = 0.0f;
        // Reset dtTarget to the current slider value, not the last controlled value
        dtTarget = apvts.getParameter("TIMESTEP")->getValue() * (0.01f - 0.0001f) + 0.0001f;
        *timestepParam = dtTarget;
        lastError = 0.0f;
        measuredFrequency = 0.0f;
        // Reset the high-pass filter's state
        for (int i = 0; i < hpf_prevInput.size(); ++i) hpf_prevInput.set(i, 0.0f);
        for (int i = 0; i < hpf_prevOutput.size(); ++i) hpf_prevOutput.set(i, 0.0f);

        analysisBuffer.clear();
    }

    // Load parameter values.
    // The level parameters were not being loaded from the APVTS correctly.
    // Let's load them now.
    const float levelX = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("LEVEL_X")->load());
    const float panX = panXParam->load();
    const float levelY = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("LEVEL_Y")->load());
    const float panY = panYParam->load();
    const float levelZ = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("LEVEL_Z")->load());
    const float panZ = panZParam->load();
    const float outputLevel = juce::Decibels::decibelsToGain(outputLevelParam->load());
    const float targetFrequency = targetFrequencyParam->load();

    // The state variables can have a large range, so we scale them down.
    // These are empirical values, you may want to adjust them.
    const float xScale = 0.025f;
    const float yScale = 0.025f;
    const float zScale = 0.0125f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // --- Apply Modulation ---
        // This is done per-sample in case the CC value changes rapidly.
        if (modTarget > 0 && modAmount != 0.0f)
        {
            // Find the parameter ID and the atomic float pointer using our maps
            auto idIt = modTargetIdMap.find(modTarget);
            auto paramIt = modTargetMap.find(modTarget);

            if (idIt != modTargetIdMap.end() && paramIt != modTargetMap.end())
            {
                const juce::String& paramId = idIt->second;
                auto* paramToMod = paramIt->second;
                auto* rangedParam = static_cast<juce::RangedAudioParameter*>(apvts.getParameter(paramId));
                const auto range = rangedParam->getNormalisableRange();

                // Get the real-world (un-normalized) value by converting the normalized value back.
                const float baseValue = range.convertFrom0to1(rangedParam->getValue());
                const float modValue = modAmount * lastCC01Value; // Bipolar modulation value

                float finalValue;
                if (modValue >= 0.0f)
                {
                    // Modulate towards max
                    finalValue = baseValue + modValue * (range.getRange().getEnd() - baseValue);
                }
                else // modValue < 0.0f
                {
                    // Modulate towards min
                    finalValue = baseValue + modValue * (baseValue - range.getRange().getStart());
                }
                paramToMod->store(finalValue);
            }
        }

        // We need a separate buffer for the pitch analysis signal
        // to ensure it's pre-fader and pre-panner.
        float pitchSourceSample = 0.0f;
        auto [x, y, z] = lorenzOsc.getNextSample();

        // --- Stability Guard ---
        // If the oscillator becomes unstable, it can return non-finite values.
        // We replace them with 0 to prevent them from corrupting the filter state.
        if (!std::isfinite(x)) x = 0.0;
        if (!std::isfinite(y)) y = 0.0;
        if (!std::isfinite(z)) z = 0.0;

        // Push points to the FIFO at a controlled rate, not on every sample.
        if (--samplesUntilNextPoint <= 0)
        {
            pushPointToFifo({(float)x, (float)y, (float)z});
            samplesUntilNextPoint = pointGenerationInterval;
        }

        // --- Pitch Source Selection ---
        // Select the signal for pitch detection *before* level and pan are applied.
        const int pitchSourceIndex = static_cast<int>(pitchSourceParam->load());
        switch (pitchSourceIndex)
        {
            case 0: pitchSourceSample = static_cast<float>(x) * xScale; break;
            case 1: pitchSourceSample = static_cast<float>(y) * yScale; break;
            case 2: pitchSourceSample = static_cast<float>(z) * zScale; break;
            default: pitchSourceSample = static_cast<float>(x) * xScale; break;
        }

        // Fill the analysis buffer for this sample.
        // We write to our temporary block-sized buffer first.
        pitchAnalysisBlock.setSample(0, sample, pitchSourceSample);

        // Scale and apply gain to each component
        const float xSample = static_cast<float>(x) * xScale * levelX;
        const float ySample = static_cast<float>(y) * yScale * levelY;
        const float zSample = static_cast<float>(z) * zScale * levelZ;

        // Apply panning (constant power)
        const float xL = xSample * juce::dsp::FastMathApproximations::cos((panX + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float xR = xSample * juce::dsp::FastMathApproximations::sin((panX + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        const float yL = ySample * juce::dsp::FastMathApproximations::cos((panY + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float yR = ySample * juce::dsp::FastMathApproximations::sin((panY + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        const float zL = zSample * juce::dsp::FastMathApproximations::cos((panZ + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float zR = zSample * juce::dsp::FastMathApproximations::sin((panZ + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

        // Get the next sample from the ADSR envelope
        const float adsrSample = ampAdsr.getNextSample();

        // Mix all sources and apply master output level
        leftChannel[sample]  = (xL + yL + zL) * outputLevel * adsrSample;
        rightChannel[sample] = (xR + yR + zR) * outputLevel * adsrSample;
    }
    
    // --- Frequency Detection & Control ---
    // First, shift the existing data in the main analysis buffer to the left
    const int numSamplesToCopy = buffer.getNumSamples();
    analysisBuffer.copyFrom(0, 0, analysisBuffer, 0, numSamplesToCopy, analysisBuffer.getNumSamples() - numSamplesToCopy);
    // Then, copy the new block of audio from our temporary buffer into the end of the main analysis buffer
    analysisBuffer.copyFrom(0, analysisBuffer.getNumSamples() - numSamplesToCopy, pitchAnalysisBlock, 0, 0, numSamplesToCopy);

    // Perform frequency detection on the block
    float freq = pitchDetector.getPitch(analysisBuffer.getReadPointer(0));
    if (freq > 0.0f) // YIN returns -1 if no frequency is detected
        measuredFrequency = freq;
    else
        measuredFrequency = 0.0f; // Indicate no frequency found

    // Now, run the PID controller based on the new measurement to determine the timestep for the *next* block.
    // Only run the PID controller if a note is being played (targetFrequency > 0)
    // and the ADSR is not in its idle state.
    if (targetFrequency > 0.0f && ampAdsr.isActive())
    {
        const float currentError = targetFrequency - measuredFrequency.load();

        // Proportional term
        const float Kp = kpParam->load();
        dtProportional = currentError;

        // Integral term
        const float Ki = kiParam->load();
        dtIntegral += currentError * Ki;
        dtIntegral = juce::jlimit(-0.001f, 0.001f, dtIntegral); // Clamp integral term to prevent wind-up

        // Derivative term
        const float Kd = kdParam->load();
        dtDerivative = currentError - lastError;
        lastError = currentError;

        // Update target dt
        dtTarget += (dtProportional * Kp) + dtIntegral + (dtDerivative * Kd);
        dtTarget = timestepRangedParam->getNormalisableRange().snapToLegalValue(dtTarget); // Clamp to the parameter's full legal range

        // Convert the real-world value back to a normalized value (0.0 - 1.0)
        auto normalizedValue = timestepRangedParam->getNormalisableRange().convertTo0to1(dtTarget);
        // Set the value and notify the host and GUI listeners
        timestepRangedParam->setValueNotifyingHost(normalizedValue);
    }

    highPassFilter(buffer, 15.0f);

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
    return new LorenzAudioProcessorEditor (*this, measuredFrequency);
}

void LorenzAudioProcessor::requestOscillatorReset()
{
    resetRequested = true;
}

void LorenzAudioProcessor::saveStateToFile()
{
    // We need to launch the file chooser asynchronously.
    // We'll use a shared_ptr to keep the FileChooser object alive until the callback is finished.
    auto fileChooser = std::make_shared<juce::FileChooser>("Save Preset",
                                                           juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                                           "*.xml",
                                                           true);

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser->launchAsync(flags, [this, fileChooser](const juce::FileChooser& chooser)
    {
        juce::File file = chooser.getResult();
        if (file != juce::File{})
        {
            // Get the current state from the APVTS.
            auto state = apvts.copyState();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());

            // Write the XML to the chosen file.
            xml->writeTo(file);
        }
    });
}

//==============================================================================
void LorenzAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // The AudioProcessorValueTreeState makes this easy by creating an XML representation of its state.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LorenzAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
            // This is useful for creating new factory presets.
            std::cout << (apvts.copyState().createXml()->toString()) << std::endl;
        }
    }
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

    std::function<juce::String(float, int)> scientificNotationStringFromValue = [](float v, int) -> juce::String
    {
        return juce::String::formatted("%.2e", v);
    };

    std::function<float(const juce::String&)> scientificNotationValueFromString = [](const juce::String& s) -> float
    {
        return s.getFloatValue();
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>("TIMESTEP", "Timestep",
                                                           juce::NormalisableRange<float>(0.0001f, 0.05f, 0.000001f, 0.5f), // Max value capped at 0.02 for stability
                                                           0.01f, juce::String(), juce::AudioProcessorParameter::genericParameter,
                                                           scientificNotationStringFromValue,
                                                           scientificNotationValueFromString));
    
    // --- ADSR Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack",
                                                           juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.1f, "s"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay",
                                                           juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.1f, "s"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain",
                                                           juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release",
                                                           juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.5f, "s"));

    // --- Modulation Parameters ---
    layout.add(std::make_unique<juce::AudioParameterChoice>("MOD_TARGET", "Mod Target",
                                                            juce::StringArray{ "Off", "Sigma", "Rho", "Beta", "Mass X", "Mass Y", "Mass Z", "Damp X", "Damp Y", "Damp Z", "Taming" },
                                                            0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("MOD_AMOUNT", "Mod Amount",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));


    // --- Frequency Control ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("TARGET_FREQ", "Target Freq",
                                                           juce::NormalisableRange<float>(0.0f, 5000.0f, 1.0f, 0.3f),
                                                           0.0f, "Hz", juce::AudioProcessorParameter::genericParameter,
                                                           std::function<juce::String(float, int)>([](float v, int) {
                                                               return (v > 0.0f) ? juce::String(v, 1) + " Hz" : "Off";
                                                           }), nullptr));

    // Use a skewed range for finer control over smaller gain values.
    // Default values are set to the original stable values.
    layout.add(std::make_unique<juce::AudioParameterFloat>("KP", "Prop. Gain",
                                                           juce::NormalisableRange<float>(0.0f, 1e-5f, 0.0f, 0.25f),
                                                           1e-6f, juce::String(), juce::AudioProcessorParameter::genericParameter,
                                                           scientificNotationStringFromValue,
                                                           scientificNotationValueFromString));
    layout.add(std::make_unique<juce::AudioParameterFloat>("KI", "Integ. Gain",
                                                           juce::NormalisableRange<float>(0.0f, 1e-6f, 0.0f, 0.25f),
                                                           2e-8f, juce::String(), juce::AudioProcessorParameter::genericParameter,
                                                           scientificNotationStringFromValue,
                                                           scientificNotationValueFromString));
    // The Kd term often needs a larger magnitude than Kp to be effective.
    layout.add(std::make_unique<juce::AudioParameterFloat>("KD", "Deriv. Gain",
                                                           juce::NormalisableRange<float>(0.0f, 1e-4f, 0.0f, 0.25f),
                                                           0.000001f, juce::String(), juce::AudioProcessorParameter::genericParameter,
                                                           scientificNotationStringFromValue,
                                                           scientificNotationValueFromString));

    layout.add(std::make_unique<juce::AudioParameterChoice>("PITCH_SOURCE", "Pitch Source",
                                                           juce::StringArray { "X", "Y", "Z" },
                                                           0)); // Default to X


    // --- Mixer Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_X", "Level X",
                                                           juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PAN_X", "Pan X",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_Y", "Level Y",
                                                           juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), -60.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PAN_Y", "Pan Y",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LEVEL_Z", "Level Z",
                                                           juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), -60.0f));
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

    layout.add(std::make_unique<juce::AudioParameterFloat>("VIEW_ZOOM_Y", "View Zoom Y",
                                                           juce::NormalisableRange<float>(10.0f, 100.0f, 0.1f, 0.5f),
                                                           50.0f));

    // --- Second Order Parameters ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("MX", "Mass X",
                                                           juce::NormalisableRange<float>(0.001f, 0.02f, 0.001f, 0.5f), 0.005f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MY", "Mass Y",
                                                           juce::NormalisableRange<float>(0.001f, 0.02f, 0.001f, 0.5f), 0.005f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MZ", "Mass Z",
                                                           juce::NormalisableRange<float>(0.001f, 0.02f, 0.001f, 0.5f), 0.005f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("CX", "Damping X",
                                                           juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CY", "Damping Y",
                                                           juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CZ", "Damping Z",
                                                           juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f), 1.0f));

    // --- Taming Parameter ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("TAMING", "Taming",
                                                           juce::NormalisableRange<float>(0.0f, 0.001f, 0.0f, 0.25f),
                                                           0.00001f, juce::String(), juce::AudioProcessorParameter::genericParameter,
                                                           scientificNotationStringFromValue,
                                                           scientificNotationValueFromString));


    return layout;
}
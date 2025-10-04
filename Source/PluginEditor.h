/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AttractorComponent.h"

//==============================================================================
/**
*/
class LorenzAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LorenzAudioProcessorEditor (LorenzAudioProcessor&);
    ~LorenzAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LorenzAudioProcessor& audioProcessor;

    fxme::FxmeLookAndFeel fxmeLookAndFeel;

    // Attractor Knobs
    fxme::FxmeKnob sigmaKnob{audioProcessor.apvts, "SIGMA", juce::Colours::red},
                    rhoKnob{audioProcessor.apvts, "RHO", juce::Colours::green},
                    betaKnob{audioProcessor.apvts, "BETA", juce::Colours::blue},
                    timestepKnob{audioProcessor.apvts, "TIMESTEP", juce::Colours::yellow};

    // X Mixer Knobs
    fxme::FxmeKnob levelXKnob{audioProcessor.apvts, "LEVEL_X", juce::Colours::red},
                    panXKnob{audioProcessor.apvts, "PAN_X", juce::Colours::blue};


    // Y Mixer Knobs
    fxme::FxmeKnob levelYKnob{audioProcessor.apvts, "LEVEL_Y", juce::Colours::green},
                    panYKnob{audioProcessor.apvts, "PAN_Y", juce::Colours::yellow};

    // Z Mixer Knobs
    fxme::FxmeKnob levelZKnob{audioProcessor.apvts, "LEVEL_Z", juce::Colours::blue},
                    panZKnob{audioProcessor.apvts, "PAN_Z", juce::Colours::red};

    // Output Knob
    fxme::FxmeKnob outputLevelKnob{audioProcessor.apvts, "OUTPUT_LEVEL", juce::Colours::white};

    AttractorComponent attractorComponent{audioProcessor};

    juce::Slider viewZoomXSlider, viewZoomZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> viewZoomXAttachment, viewZoomZAttachment;

    juce::Label viewZoomXLabel, viewZoomZLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessorEditor)
};

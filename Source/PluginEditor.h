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

    // Duffing Knobs
    fxme::FxmeKnob alphaKnob{audioProcessor.apvts, "ALPHA", juce::Colours::red},
                    betaKnob{audioProcessor.apvts, "BETA_DUFFING", juce::Colours::green},
                    gammaKnob{audioProcessor.apvts, "GAMMA", juce::Colours::blue},
                    deltaKnob{audioProcessor.apvts, "DELTA", juce::Colours::cyan},
                    frequencyKnob{audioProcessor.apvts, "FREQUENCY", juce::Colours::magenta},
                    timestepKnob{audioProcessor.apvts, "TIMESTEP", juce::Colours::yellow};

    // Output Knob
    fxme::FxmeKnob outputLevelKnob{audioProcessor.apvts, "OUTPUT_LEVEL", juce::Colours::white};

    AttractorComponent attractorComponent{audioProcessor};

    juce::Slider viewZoomXSlider, viewZoomZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> viewZoomXAttachment, viewZoomZAttachment;

    juce::Label viewZoomXLabel, viewZoomZLabel;

    juce::TextButton resetButton { "Reset Oscillator" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessorEditor)
};

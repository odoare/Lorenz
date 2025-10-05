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
class LorenzAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    LorenzAudioProcessorEditor (LorenzAudioProcessor&, std::atomic<float>& measuredFreq);
    ~LorenzAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LorenzAudioProcessor& audioProcessor;
    // Reference to the measured frequency in the processor
    std::atomic<float>& measuredFrequency;

    fxme::FxmeLookAndFeel fxmeLookAndFeel;

    // Attractor Knobs
    fxme::FxmeKnob sigmaKnob{audioProcessor.apvts, "SIGMA", juce::Colours::red},
                    rhoKnob{audioProcessor.apvts, "RHO", juce::Colours::green},
                    betaKnob{audioProcessor.apvts, "BETA", juce::Colours::blue},
                    timestepKnob{audioProcessor.apvts, "TIMESTEP", juce::Colours::yellow};
    // Frequency Control
    fxme::FxmeKnob targetFrequencyKnob{audioProcessor.apvts, "TARGET_FREQ", juce::Colours::orange};
    fxme::FxmeKnob kpKnob{audioProcessor.apvts, "KP", juce::Colours::cyan};
    fxme::FxmeKnob kiKnob{audioProcessor.apvts, "KI", juce::Colours::magenta};
    fxme::FxmeKnob kdKnob{audioProcessor.apvts, "KD", juce::Colours::lightgreen};
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

    // Second Order Knobs
    fxme::FxmeKnob mxKnob{audioProcessor.apvts, "MX", juce::Colours::red},
                    myKnob{audioProcessor.apvts, "MY", juce::Colours::green},
                    mzKnob{audioProcessor.apvts, "MZ", juce::Colours::blue},
                    cxKnob{audioProcessor.apvts, "CX", juce::Colours::red.brighter(0.5f)},
                    cyKnob{audioProcessor.apvts, "CY", juce::Colours::green.brighter(0.5f)},
                    czKnob{audioProcessor.apvts, "CZ", juce::Colours::blue.brighter(0.5f)};

    // Taming Knob
    fxme::FxmeKnob tamingKnob{audioProcessor.apvts, "TAMING", juce::Colours::purple};

    AttractorComponent attractorComponent{audioProcessor};

    juce::Slider viewZoomXSlider, viewZoomZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> viewZoomXAttachment, viewZoomZAttachment;

    juce::Label viewZoomXLabel, viewZoomZLabel;
    juce::Label measuredFrequencyLabel;

    juce::ComboBox pitchSourceSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchSourceAttachment;
    juce::Label pitchSourceLabel;

    juce::TextButton resetButton { "Reset Oscillator" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessorEditor)
};

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
    fxme::FxmeKnob sigmaKnob{audioProcessor.apvts, "SIGMA", "SIGMA", juce::Colours::cyan},
                    rhoKnob{audioProcessor.apvts, "RHO", "RHO", juce::Colours::cyan},
                    betaKnob{audioProcessor.apvts, "BETA", "BETA", juce::Colours::cyan},
                    timestepKnob{audioProcessor.apvts, "TIMESTEP", "TIMESTEP", juce::Colours::orange};
    // Frequency Control
    fxme::FxmeKnob targetFrequencyKnob{audioProcessor.apvts, "TARGET_FREQ", "TARGET_FREQ", juce::Colours::orange};
    fxme::FxmeKnob kpKnob{audioProcessor.apvts, "KP", "KP", juce::Colours::lightgreen};
    fxme::FxmeKnob kiKnob{audioProcessor.apvts, "KI", "KI", juce::Colours::lightgreen};
    fxme::FxmeKnob kdKnob{audioProcessor.apvts, "KD", "KD", juce::Colours::lightgreen};

    // ADSR Knobs
    fxme::FxmeKnob attackKnob{audioProcessor.apvts, "ATTACK", "ATTACK", juce::Colours::yellow};
    fxme::FxmeKnob decayKnob{audioProcessor.apvts, "DECAY", "DECAY", juce::Colours::yellow};
    fxme::FxmeKnob sustainKnob{audioProcessor.apvts, "SUSTAIN", "SUSTAIN", juce::Colours::yellow};
    fxme::FxmeKnob releaseKnob{audioProcessor.apvts, "RELEASE", "RELEASE", juce::Colours::yellow};

    // Modulation Controls
    juce::ComboBox modTargetSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modTargetAttachment;
    juce::Label modTargetLabel;
    fxme::FxmeKnob modAmountKnob{audioProcessor.apvts, "MOD_AMOUNT", "MOD_AMOUNT", juce::Colours::magenta};

    // X Mixer Knobs
    fxme::FxmeKnob levelXKnob{audioProcessor.apvts, "LEVEL_X", "LEVEL_X", juce::Colours::red.brighter(0.7f)},
                    panXKnob{audioProcessor.apvts, "PAN_X", "PAN_X", juce::Colours::red.brighter(0.9f)};


    // Y Mixer Knobs
    fxme::FxmeKnob levelYKnob{audioProcessor.apvts, "LEVEL_Y", "LEVEL_Y", juce::Colours::green.brighter(0.7f)},
                    panYKnob{audioProcessor.apvts, "PAN_Y", "PAN_Y", juce::Colours::green.brighter(0.9f)};

    // Z Mixer Knobs
    fxme::FxmeKnob levelZKnob{audioProcessor.apvts, "LEVEL_Z", "LEVEL_Z", juce::Colours::blue.brighter(0.7f)},
                    panZKnob{audioProcessor.apvts, "PAN_Z", "PAN_Z", juce::Colours::blue.brighter(0.9f)};

    // Output Knob
    fxme::FxmeKnob outputLevelKnob{audioProcessor.apvts, "OUTPUT_LEVEL", "OUTPUT_LEVEL", juce::Colours::white};

    // Second Order Knobs
    fxme::FxmeKnob mxKnob{audioProcessor.apvts, "MX", "MX", juce::Colours::red},
                    myKnob{audioProcessor.apvts, "MY", "MY", juce::Colours::green},
                    mzKnob{audioProcessor.apvts, "MZ", "MZ", juce::Colours::blue},
                    cxKnob{audioProcessor.apvts, "CX", "CX", juce::Colours::red.brighter(0.5f)},
                    cyKnob{audioProcessor.apvts, "CY", "CY", juce::Colours::green.brighter(0.5f)},
                    czKnob{audioProcessor.apvts, "CZ", "CZ", juce::Colours::blue.brighter(0.5f)};

    // Taming Knob
    fxme::FxmeKnob tamingKnob{audioProcessor.apvts, "TAMING", "TAMING", juce::Colours::purple};

    AttractorComponent attractorComponent{audioProcessor};

    juce::Slider viewZoomXSlider, viewZoomZSlider, viewZoomYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> viewZoomXAttachment, viewZoomZAttachment, viewZoomYAttachment;

    juce::Label viewZoomXLabel, viewZoomZLabel, viewZoomYLabel;
    juce::Label measuredFrequencyLabel;

    juce::ComboBox pitchSourceSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchSourceAttachment;
    juce::Label pitchSourceLabel;

    juce::TextButton resetButton { "Reset Oscillator" };
    juce::TextButton savePresetButton { "Save Preset" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessorEditor)
};

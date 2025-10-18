/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AttractorComponent3D.h"

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
    fxme::FxmeKnob sigmaKnob{audioProcessor.apvts, "SIGMA", "Sigma", juce::Colours::cyan},
                    rhoKnob{audioProcessor.apvts, "RHO", "Rho", juce::Colours::cyan},
                    betaKnob{audioProcessor.apvts, "BETA", "Beta", juce::Colours::cyan},
                    timestepKnob{audioProcessor.apvts, "TIMESTEP", "Timestep", juce::Colours::orange};
    // Frequency Control
    fxme::FxmeKnob targetFrequencyKnob{audioProcessor.apvts, "TARGET_FREQ", "Target freq.", juce::Colours::orange};
    fxme::FxmeKnob kpKnob{audioProcessor.apvts, "KP", "Kp", juce::Colours::lightgreen};
    fxme::FxmeKnob kiKnob{audioProcessor.apvts, "KI", "Ki", juce::Colours::lightgreen};
    fxme::FxmeKnob kdKnob{audioProcessor.apvts, "KD", "Kd", juce::Colours::lightgreen};
    fxme::FxmeKnob pidIntervalKnob{audioProcessor.apvts, "PID_INTERVAL", "PID Int.", juce::Colours::orange};

    // ADSR Knobs
    fxme::FxmeKnob attackKnob{audioProcessor.apvts, "ATTACK", "A", juce::Colours::yellow};
    fxme::FxmeKnob decayKnob{audioProcessor.apvts, "DECAY", "D", juce::Colours::yellow};
    fxme::FxmeKnob sustainKnob{audioProcessor.apvts, "SUSTAIN", "S", juce::Colours::yellow};
    fxme::FxmeKnob releaseKnob{audioProcessor.apvts, "RELEASE", "R", juce::Colours::yellow};

    // Modulation Controls
    juce::ComboBox modTargetSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modTargetAttachment;
    juce::Label modTargetLabel;
    fxme::FxmeKnob modAmountKnob{audioProcessor.apvts, "MOD_AMOUNT", "", juce::Colours::magenta};

    // X Mixer Knobs
    fxme::FxmeKnob levelXKnob{audioProcessor.apvts, "LEVEL_X", "Level X", juce::Colours::red.brighter(0.7f)},
                    panXKnob{audioProcessor.apvts, "PAN_X", "Pan X", juce::Colours::red.brighter(0.9f)};


    // Y Mixer Knobs
    fxme::FxmeKnob levelYKnob{audioProcessor.apvts, "LEVEL_Y", "Level Y", juce::Colours::green.brighter(0.7f)},
                    panYKnob{audioProcessor.apvts, "PAN_Y", "Pan Y", juce::Colours::green.brighter(0.9f)};

    // Z Mixer Knobs
    fxme::FxmeKnob levelZKnob{audioProcessor.apvts, "LEVEL_Z", "Level Z", juce::Colours::blue.brighter(0.7f)},
                    panZKnob{audioProcessor.apvts, "PAN_Z", "Pan Z", juce::Colours::blue.brighter(0.9f)};

    // Output Knob
    fxme::FxmeKnob outputLevelKnob{audioProcessor.apvts, "OUTPUT_LEVEL", "Output", juce::Colours::white};

    // Second Order Knobs
    fxme::FxmeKnob mxKnob{audioProcessor.apvts, "MX", "Mx", juce::Colours::red},
                    myKnob{audioProcessor.apvts, "MY", "My", juce::Colours::green},
                    mzKnob{audioProcessor.apvts, "MZ", "Mz", juce::Colours::blue},
                    cxKnob{audioProcessor.apvts, "CX", "Cx", juce::Colours::red.brighter(0.5f)},
                    cyKnob{audioProcessor.apvts, "CY", "Cy", juce::Colours::green.brighter(0.5f)},
                    czKnob{audioProcessor.apvts, "CZ", "Cz", juce::Colours::blue.brighter(0.5f)};

    // Taming Knob
    fxme::FxmeKnob tamingKnob{audioProcessor.apvts, "TAMING", "Taming", juce::Colours::purple};

    AttractorComponent3D attractorComponent{audioProcessor};

    juce::Slider viewZoomXSlider, viewZoomZSlider, viewZoomYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> viewZoomXAttachment, viewZoomZAttachment, viewZoomYAttachment;

    juce::Label viewZoomXLabel, viewZoomZLabel, viewZoomYLabel;
    juce::Label measuredFrequencyLabel;

    juce::ComboBox pitchSourceSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchSourceAttachment;
    juce::Label pitchSourceLabel;

    juce::TextButton resetButton { "Reset" };
    juce::TextButton savePresetButton { "Save" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessorEditor)
};

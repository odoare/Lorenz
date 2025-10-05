/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LorenzAudioProcessorEditor::LorenzAudioProcessorEditor (LorenzAudioProcessor& p, std::atomic<float>& measuredFreq)
    : AudioProcessorEditor (&p), audioProcessor (p), measuredFrequency(measuredFreq)
{
    auto& apvts = audioProcessor.apvts;

    addAndMakeVisible(sigmaKnob);
    sigmaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(rhoKnob);
    rhoKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(betaKnob);
    betaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(timestepKnob);
    timestepKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(targetFrequencyKnob);
    targetFrequencyKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(kpKnob);
    kpKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(kiKnob);
    kiKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(kdKnob);
    kdKnob.slider.setLookAndFeel(&fxmeLookAndFeel);


    addAndMakeVisible(levelXKnob);
    levelXKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(panXKnob);
    panXKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(levelYKnob);
    levelYKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(panYKnob);
    panYKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(levelZKnob);
    levelZKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(panZKnob);
    panZKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    
    addAndMakeVisible(outputLevelKnob);
    outputLevelKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    
    addAndMakeVisible(mxKnob);
    mxKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(myKnob);
    myKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(mzKnob);
    mzKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(cxKnob);
    cxKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(cyKnob);
    cyKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(czKnob);
    czKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(tamingKnob);
    tamingKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(attractorComponent);

    addAndMakeVisible(viewZoomXSlider);
    viewZoomXSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    viewZoomXSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    viewZoomXAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "VIEW_ZOOM_X", viewZoomXSlider);

    addAndMakeVisible(viewZoomXLabel);
    viewZoomXLabel.setText("X Zoom", juce::dontSendNotification);
    viewZoomXLabel.attachToComponent(&viewZoomXSlider, false);
    viewZoomXLabel.setJustificationType(juce::Justification::centredTop);

    addAndMakeVisible(viewZoomZSlider);
    viewZoomZSlider.setSliderStyle(juce::Slider::LinearVertical);
    viewZoomZSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    viewZoomZAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "VIEW_ZOOM_Z", viewZoomZSlider);

    addAndMakeVisible(viewZoomZLabel);
    viewZoomZLabel.setText("Z Zoom", juce::dontSendNotification);
    viewZoomZLabel.attachToComponent(&viewZoomZSlider, true);
    viewZoomZLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(resetButton);
    resetButton.onClick = [this]
    {
        audioProcessor.requestOscillatorReset();
    };

    addAndMakeVisible(measuredFrequencyLabel);
    measuredFrequencyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    measuredFrequencyLabel.setJustificationType(juce::Justification::centred);
    measuredFrequencyLabel.setText("--- Hz", juce::dontSendNotification);

    addAndMakeVisible(pitchSourceSelector);
    // We must manually populate the ComboBox with the choices from the parameter.
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("PITCH_SOURCE")))
        pitchSourceSelector.addItemList(choiceParam->choices, 1);

    pitchSourceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "PITCH_SOURCE", pitchSourceSelector);

    addAndMakeVisible(pitchSourceLabel);
    pitchSourceLabel.setText("Pitch Source", juce::dontSendNotification);
    pitchSourceLabel.attachToComponent(&pitchSourceSelector, true);
    pitchSourceLabel.setJustificationType(juce::Justification::centred);

    startTimerHz(30); // Update the frequency display 30 times per second

    setSize (1280, 800);
}

LorenzAudioProcessorEditor::~LorenzAudioProcessorEditor()
{
    stopTimer();
}

void LorenzAudioProcessorEditor::timerCallback()
{
    float freq = measuredFrequency.load();
    juce::String freqText = (freq > 0.0f) ? juce::String(freq, 1) + " Hz" : "--- Hz";
    measuredFrequencyLabel.setText(freqText, juce::dontSendNotification);
}

//==============================================================================
void LorenzAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void LorenzAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Define a top area for the main controls and a bottom area for the new knobs
    auto bottomArea = bounds.removeFromBottom(120);
    auto topArea = bounds;

    const int knobSize = 80;
    const int knobSpacing = 100;
    const int groupSpacing = 30;

    // Attractor Parameters Column
    auto attractorBounds = topArea.removeFromLeft(knobSpacing);
    sigmaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    rhoKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    betaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    timestepKnob.setBounds(attractorBounds.removeFromTop(knobSize).withY(betaKnob.getBottom()));
    resetButton.setBounds(attractorBounds.getX(), timestepKnob.getBottom() + 10, attractorBounds.getWidth(), 24);

    auto rightSide = topArea.removeFromRight(topArea.getWidth() / 2);
    auto attractorViewBounds = rightSide.reduced(10);

    viewZoomZSlider.setBounds(attractorViewBounds.removeFromRight(20));
    viewZoomXSlider.setBounds(attractorViewBounds.removeFromBottom(20));

    attractorComponent.setBounds(attractorViewBounds);

    auto leftSide = topArea;
    leftSide.removeFromLeft(groupSpacing);

    // --- Frequency Control Column ---
    auto freqControlBounds = leftSide.removeFromLeft(knobSpacing);
    measuredFrequencyLabel.setBounds(freqControlBounds.getX(), betaKnob.getY() - 20, freqControlBounds.getWidth(), 20);
    targetFrequencyKnob.setBounds(freqControlBounds.getX(), betaKnob.getY(), knobSize, knobSize);
    kpKnob.setBounds(freqControlBounds.getX(), targetFrequencyKnob.getBottom(), knobSize / 2, knobSize / 2);
    kiKnob.setBounds(freqControlBounds.getX() + knobSize / 2, targetFrequencyKnob.getBottom(), knobSize / 2, knobSize / 2);
    kdKnob.setBounds(freqControlBounds.getX(), kpKnob.getBottom(), knobSize, knobSize / 2);
    pitchSourceSelector.setBounds(freqControlBounds.getX(), kdKnob.getBottom() + 10, knobSize, 24);


    // Mixer Columns
    auto mixerXBounds = leftSide.removeFromLeft(knobSpacing);
    levelXKnob.setBounds(mixerXBounds.removeFromTop(knobSize));
    panXKnob.setBounds(mixerXBounds.removeFromTop(knobSize));

    auto mixerYBounds = leftSide.removeFromLeft(knobSpacing);
    levelYKnob.setBounds(mixerYBounds.removeFromTop(knobSize));
    panYKnob.setBounds(mixerYBounds.removeFromTop(knobSize));

    auto mixerZBounds = leftSide.removeFromLeft(knobSpacing);
    levelZKnob.setBounds(mixerZBounds.removeFromTop(knobSize));
    panZKnob.setBounds(mixerZBounds.removeFromTop(knobSize));

    leftSide.removeFromLeft(groupSpacing);

    // Output Column
    auto outputBounds = leftSide.removeFromLeft(knobSpacing);
    outputLevelKnob.setBounds(outputBounds.withSize(knobSize, knobSize).withY(0));

    // --- Second Order Parameters (Bottom Row) ---
    bottomArea.removeFromLeft(groupSpacing); // Add some padding
    mxKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
    myKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
    mzKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
    bottomArea.removeFromLeft(groupSpacing); // Space between groups
    cxKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
    cyKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
    czKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));

    bottomArea.removeFromLeft(groupSpacing);
    tamingKnob.setBounds(bottomArea.removeFromLeft(knobSpacing).withSize(knobSize, knobSize));
}

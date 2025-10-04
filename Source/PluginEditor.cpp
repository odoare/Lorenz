/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LorenzAudioProcessorEditor::LorenzAudioProcessorEditor (LorenzAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
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

    setSize (1024, 600);
}

LorenzAudioProcessorEditor::~LorenzAudioProcessorEditor()
{
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

    const int knobSize = 80;
    const int knobSpacing = 100;
    const int groupSpacing = 30;

    // Attractor Parameters Column
    auto attractorBounds = bounds.removeFromLeft(knobSpacing);
    sigmaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    rhoKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    betaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    timestepKnob.setBounds(attractorBounds.removeFromTop(knobSize));

    auto rightSide = bounds.removeFromRight(bounds.getWidth() / 2);
    auto attractorViewBounds = rightSide.reduced(10);

    viewZoomZSlider.setBounds(attractorViewBounds.removeFromRight(20));
    viewZoomXSlider.setBounds(attractorViewBounds.removeFromBottom(20));

    attractorComponent.setBounds(attractorViewBounds);

    auto leftSide = bounds;
    leftSide.removeFromLeft(groupSpacing);

    // Mixer Columns
    auto mixerBounds = leftSide.removeFromLeft(knobSpacing * 3);
    auto xBounds = mixerBounds.removeFromLeft(knobSpacing);
    levelXKnob.setBounds(xBounds.removeFromTop(knobSize));
    panXKnob.setBounds(xBounds.removeFromTop(knobSize));
    
    auto yBounds = mixerBounds.removeFromLeft(knobSpacing);
    levelYKnob.setBounds(yBounds.removeFromTop(knobSize));
    panYKnob.setBounds(yBounds.removeFromTop(knobSize));
    
    auto zBounds = mixerBounds.removeFromLeft(knobSpacing);
    levelZKnob.setBounds(zBounds.removeFromTop(knobSize));
    panZKnob.setBounds(zBounds.removeFromTop(knobSize));
    
    leftSide.removeFromLeft(groupSpacing);

    // Output Column
    auto outputBounds = leftSide.removeFromLeft(knobSpacing);
    outputLevelKnob.setBounds(outputBounds.withSize(knobSize, knobSize).withY(0));
}

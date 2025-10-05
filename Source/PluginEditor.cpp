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

    addAndMakeVisible(alphaKnob);
    alphaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(betaKnob);
    betaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(gammaKnob);
    gammaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(deltaKnob);
    deltaKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(frequencyKnob);
    frequencyKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(timestepKnob);
    timestepKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    
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
    viewZoomZLabel.setText("V Zoom", juce::dontSendNotification);
    viewZoomZLabel.attachToComponent(&viewZoomZSlider, true);
    viewZoomZLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(resetButton);
    resetButton.onClick = [this]
    {
        audioProcessor.requestOscillatorReset();
    };

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
    
    auto topArea = bounds;

    const int knobSize = 80;
    const int knobSpacing = 100;
    const int groupSpacing = 30;

    // Duffing Parameters Column
    auto attractorBounds = topArea.removeFromLeft(knobSpacing);
    alphaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    betaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    gammaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    deltaKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    frequencyKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    timestepKnob.setBounds(attractorBounds.removeFromTop(knobSize));
    resetButton.setBounds(attractorBounds.getX(), timestepKnob.getBottom() + 10, attractorBounds.getWidth(), 24);

    auto rightSide = topArea.removeFromRight(topArea.getWidth() / 2);
    auto attractorViewBounds = rightSide.reduced(10);

    viewZoomZSlider.setBounds(attractorViewBounds.removeFromRight(20));
    viewZoomXSlider.setBounds(attractorViewBounds.removeFromBottom(20));

    attractorComponent.setBounds(attractorViewBounds);

    auto leftSide = topArea;
    leftSide.removeFromLeft(groupSpacing);

    leftSide.removeFromLeft(groupSpacing);

    // Output Column
    auto outputBounds = leftSide.removeFromLeft(knobSpacing);
    outputLevelKnob.setBounds(outputBounds.withSize(knobSize, knobSize).withY(0));
}

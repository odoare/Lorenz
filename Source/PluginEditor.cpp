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

    addAndMakeVisible(attackKnob);
    attackKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(decayKnob);
    decayKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(sustainKnob);
    sustainKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(releaseKnob);
    releaseKnob.slider.setLookAndFeel(&fxmeLookAndFeel);

    addAndMakeVisible(modAmountKnob);
    modAmountKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    addAndMakeVisible(modTargetSelector);
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("MOD_TARGET")))
        modTargetSelector.addItemList(choiceParam->choices, 1);
    modTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "MOD_TARGET", modTargetSelector);
    addAndMakeVisible(modTargetLabel);
    modTargetLabel.setText("CC01 Mod Target", juce::dontSendNotification);
    modTargetLabel.setJustificationType(juce::Justification::centred);

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
    viewZoomXLabel.setText("X", juce::dontSendNotification);
    // viewZoomXLabel.attachToComponent(&viewZoomXSlider, false);
    // viewZoomXLabel.setJustificationType(juce::Justification::centredTop);

    addAndMakeVisible(viewZoomZSlider);
    viewZoomZSlider.setSliderStyle(juce::Slider::LinearVertical);
    viewZoomZSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    viewZoomZAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "VIEW_ZOOM_Z", viewZoomZSlider);

    addAndMakeVisible(viewZoomZLabel);
    viewZoomZLabel.setText("Z", juce::dontSendNotification);
    // viewZoomZLabel.attachToComponent(&viewZoomZSlider, true);
    // viewZoomZLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(viewZoomYSlider);
    viewZoomYSlider.setSliderStyle(juce::Slider::LinearVertical);
    viewZoomYSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    viewZoomYAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "VIEW_ZOOM_Y", viewZoomYSlider);

    addAndMakeVisible(viewZoomYLabel);
    viewZoomYLabel.setText("Y", juce::dontSendNotification);
    // viewZoomYLabel.attachToComponent(&viewZoomYSlider, true);
    // viewZoomYLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(resetButton);
    resetButton.onClick = [this]
    {
        audioProcessor.requestOscillatorReset();
    };

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this]
    {
        audioProcessor.saveStateToFile();
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
    //pitchSourceLabel.attachToComponent(&pitchSourceSelector, true);
    pitchSourceLabel.setJustificationType(juce::Justification::centred);

    startTimerHz(30); // Update the frequency display 30 times per second

    setSize (800, 600);
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
    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (270.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;
    auto bluegreengrey = juce::Colour::fromFloatRGBA (0.15f, 0.15f, 0.25f, 1.0f);
    juce::ColourGradient grad (bluegreengrey.darker().darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();
}

void LorenzAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    using fi = juce::FlexItem; // Using alias for brevity
    
    juce::FlexBox fbLorenz, fbFreqControl, fbOutput, fbMiddle, fbGraphx, fbGraphy, fbGraphx2;
    juce::FlexBox fbL1, fbL2, fbL3, fbL4, fbL5;
    juce::FlexBox fbF1, fbF2, fbF11, fbMod;
    juce::FlexBox fbMain;
    juce::FlexBox fbAdsr;
    
    fbL1.flexDirection = juce::FlexBox::Direction::row;
    fbL2.flexDirection = juce::FlexBox::Direction::row;
    fbL3.flexDirection = juce::FlexBox::Direction::row;
    fbL4.flexDirection = juce::FlexBox::Direction::row;
    fbL5.flexDirection = juce::FlexBox::Direction::row;
    fbF1.flexDirection = juce::FlexBox::Direction::row;
    fbMod.flexDirection = juce::FlexBox::Direction::column;
    fbF11.flexDirection = juce::FlexBox::Direction::column;
    fbF2.flexDirection = juce::FlexBox::Direction::row;
    fbMain.flexDirection = juce::FlexBox::Direction::row;

    fbGraphx.flexDirection = juce::FlexBox::Direction::row;
    fbGraphy.flexDirection = juce::FlexBox::Direction::column;
    fbGraphx2.flexDirection = juce::FlexBox::Direction::row;

    fbLorenz.flexDirection = juce::FlexBox::Direction::column;
    fbFreqControl.flexDirection = juce::FlexBox::Direction::column;
    fbOutput.flexDirection = juce::FlexBox::Direction::row;
    fbMiddle.flexDirection = juce::FlexBox::Direction::column;

    fbL1.items.add(fi(sigmaKnob).withFlex(1.f));
    fbL1.items.add(fi(rhoKnob).withFlex(1.f));
    fbL1.items.add(fi(betaKnob).withFlex(1.f));
    fbL2.items.add(fi(mxKnob).withFlex(1.f));
    fbL2.items.add(fi(myKnob).withFlex(1.f));
    fbL2.items.add(fi(mzKnob).withFlex(1.f));
    fbL3.items.add(fi(cxKnob).withFlex(1.f));
    fbL3.items.add(fi(cyKnob).withFlex(1.f));
    fbL3.items.add(fi(czKnob).withFlex(1.f));
    fbL4.items.add(fi(levelXKnob).withFlex(1.f));
    fbL4.items.add(fi(levelYKnob).withFlex(1.f));
    fbL4.items.add(fi(levelZKnob).withFlex(1.f));
    fbL5.items.add(fi(panXKnob).withFlex(1.f));
    fbL5.items.add(fi(panYKnob).withFlex(1.f));
    fbL5.items.add(fi(panZKnob).withFlex(1.f));
    fbLorenz.items.add(fi(fbL1).withFlex(1.f));
    fbLorenz.items.add(fi(fbL2).withFlex(1.f));
    fbLorenz.items.add(fi(fbL3).withFlex(1.f));
    fbLorenz.items.add(fi(fbL4).withFlex(1.f));
    fbLorenz.items.add(fi(fbL5).withFlex(1.f));
    fbF1.items.add(fi(targetFrequencyKnob).withFlex(1.f));
    fbF1.items.add(fi(timestepKnob).withFlex(1.f));
    fbF11.items.add(fi(pitchSourceLabel).withFlex(1.f));
    fbF11.items.add(fi(pitchSourceSelector).withFlex(1.f).withMargin(10).withMargin(juce::FlexItem::Margin(0,20,10,20)));
    fbF11.items.add(fi(measuredFrequencyLabel).withFlex(1.f));
    fbF1.items.add(fi(fbF11).withFlex(1.f));
    fbF1.items.add(fi(kpKnob).withFlex(1.f));
    fbF1.items.add(fi(kiKnob).withFlex(1.f));
    fbF1.items.add(fi(kdKnob).withFlex(1.f));
    fbAdsr.items.add(fi(attackKnob).withFlex(1.f));
    fbAdsr.items.add(fi(decayKnob).withFlex(1.f));
    fbAdsr.items.add(fi(sustainKnob).withFlex(1.f));
    fbAdsr.items.add(fi(releaseKnob).withFlex(1.f));
    fbMod.items.add(fi(modTargetLabel).withFlex(1.f));
    fbMod.items.add(fi(modTargetSelector).withFlex(1.f).withMargin(juce::FlexItem::Margin(0, 0, 10, 0)));
    fbMod.items.add(fi(modAmountKnob).withFlex(2.f));

    juce::FlexBox fbButtons;
    fbButtons.flexDirection = juce::FlexBox::Direction::column;
    fbButtons.items.add(fi(savePresetButton).withFlex(1.f).withMargin(juce::FlexItem::Margin(5, 10, 5, 10)));
    fbButtons.items.add(fi(resetButton).withFlex(1.f).withMargin(juce::FlexItem::Margin(5, 10, 5, 10)));

    fbOutput.items.add(fi(fbAdsr).withFlex(4.f).withMargin(juce::FlexItem::Margin(20,0,10,0)));
    fbOutput.items.add(fi(fbMod).withFlex(1.5f).withMargin(juce::FlexItem::Margin(0, 0, 0, 20)));
    fbOutput.items.add(fi(outputLevelKnob).withFlex(2.f).withMargin(juce::FlexItem::Margin(10,0,10,10)));
    fbOutput.items.add(fi(tamingKnob).withFlex(2.f).withMargin(juce::FlexItem::Margin(10,0,10,10)));
    fbOutput.items.add(fi(fbButtons).withFlex(1.5f).withMargin(juce::FlexItem::Margin(20, 0, 20, 0)));
    fbGraphx.items.add(fi(attractorComponent).withFlex(1.f));
    fbGraphx.items.add(fi(viewZoomZSlider).withFlex(.05f));
    fbGraphx.items.add(fi(viewZoomYSlider).withFlex(.05f));
    fbGraphx2.items.add(fi(viewZoomXLabel).withFlex(.05f));
    fbGraphx2.items.add(fi(viewZoomXLabel).withFlex(.05f));
    fbGraphx2.items.add(fi(viewZoomXSlider).withFlex(.95f));
    fbGraphx2.items.add(fi(viewZoomYLabel).withFlex(.05f));
    fbGraphx2.items.add(fi(viewZoomZLabel).withFlex(.05f));
    fbGraphy.items.add(fi(fbGraphx).withFlex(1.f));
    fbGraphy.items.add(fi(fbGraphx2).withFlex(.1f));
    fbMiddle.items.add(fi(fbF1).withFlex(.9f));
    fbMiddle.items.add(fi(fbOutput).withFlex(1.1f));
    fbMiddle.items.add(fi(fbGraphy).withFlex(3.f));
    fbMain.items.add(fi(fbLorenz).withFlex(1.f));
    fbMain.items.add(fi(fbMiddle).withFlex(2.f));

    fbMain.performLayout(bounds);
}

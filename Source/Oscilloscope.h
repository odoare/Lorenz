/*
  ==============================================================================

    Oscilloscope.h
    Created: 1 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * A simple scrolling oscilloscope component.
*/
class Oscilloscope : public juce::Component
{
public:
    Oscilloscope(const juce::String& title, juce::Colour traceColour)
        : scopeTitle(title),
          colour(traceColour)
    {
        buffer.resize(bufferSize, 0.0f);
    }

    void addSample(float sample)
    {
        // Scale the incoming sample to a more viewable range.
        // This is an empirical value, you might want to adjust it.
        const float inputGain = 0.05f;
        buffer[writeIndex] = sample * inputGain;
        writeIndex = (writeIndex + 1) % bufferSize;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1.0f);

        g.setColour(juce::Colours::white);
        g.drawText(scopeTitle, getLocalBounds().reduced(5), juce::Justification::topLeft, false);

        auto bounds = getLocalBounds().toFloat();
        const float midY = bounds.getCentreY();
        const float gain = bounds.getHeight() / 2.0f;

        g.setColour(colour);
        juce::Path path;

        // Start path from the oldest sample
        path.startNewSubPath(0.0f, midY - buffer[writeIndex] * gain);

        for (int i = 1; i < bufferSize; ++i)
        {
            const int readIndex = (writeIndex + i) % bufferSize;
            const float x = juce::jmap((float)i, 0.0f, (float)bufferSize, bounds.getX(), bounds.getRight());
            const float y = midY - buffer[readIndex] * gain;
            path.lineTo(x, y);
        }

        g.strokePath(path, juce::PathStrokeType(1.5f));
    }

private:
    static constexpr int bufferSize = 512;
    std::vector<float> buffer;
    int writeIndex = 0;
    juce::String scopeTitle;
    juce::Colour colour;
};
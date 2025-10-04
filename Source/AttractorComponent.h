/*
  ==============================================================================

    AttractorComponent.h
    Created: 1 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * A component to visualize the Lorenz attractor's path.
 * It reads 3D points from a shared FIFO and draws a 2D projection of the path.
*/
class AttractorComponent  : public juce::Component, private juce::Timer
{
public:
    AttractorComponent(LorenzAudioProcessor& p) : audioProcessor(p)
    {
        points.reserve(maxPathPoints);
        // Start the timer to update the display. 50Hz is a good rate for smooth animation.
        startTimerHz(50);
    }

    ~AttractorComponent() override
    {
        stopTimer();
    }

    void paint (juce::Graphics& g) override
    {
        // Fill background
        g.fillAll (juce::Colours::black);

        // Create the path from our points container
        juce::Path path;
        if (! points.empty())
        {
            path.startNewSubPath (points.front());
            for (size_t i = 1; i < points.size(); ++i)
            {
                path.lineTo (points[i]);
            }
        }

        // Set path color and stroke
        g.setColour (juce::Colours::white.withAlpha(0.7f));
        g.strokePath (path, juce::PathStrokeType(1.5f));

        // Draw a border
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        // When the component is resized, we should recalculate the path
        // with the new dimensions. We can clear the points to force a redraw.
        points.clear();
    }

private:
    void timerCallback() override
    {
        const int maxPointsPerTimerCall = 200;
        const auto bounds = getLocalBounds().toFloat();

        // Get zoom values from parameters. These represent the width/height of the view.
        const float viewWidth = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_X");
        const float viewHeight = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Z");

        // For now, we'll keep the view centered.
        const float centerX = 0.0f;
        const float centerZ = 25.0f;

        const float lorenzXMin = centerX - viewWidth / 2.0f;
        const float lorenzXMax = centerX + viewWidth / 2.0f;
        const float lorenzZMin = centerZ - viewHeight / 2.0f;
        const float lorenzZMax = centerZ + viewHeight / 2.0f;

        for (int i = 0; i < maxPointsPerTimerCall; ++i)
        {
            LorenzAudioProcessor::Point p;
            if (audioProcessor.getPointFromFifo(p))
            {
                // Map Lorenz X coordinate to component's X-axis.
                const float screenX = juce::jmap (p.x, lorenzXMin, lorenzXMax, bounds.getX(), bounds.getRight());
                
                // Map Lorenz Z coordinate to component's Y-axis.
                // We map the Lorenz Z range to the full height, and invert the Y-axis so higher Z is higher on screen.
                const float screenZ = juce::jmap (p.z, lorenzZMin, lorenzZMax, bounds.getBottom(), bounds.getY());
                
                points.push_back({screenX, screenZ});
            }
            else
            {
                // No more points in the FIFO for now
                break;
            }
        }
        
        // To prevent the path from growing indefinitely and consuming too much memory,
        // we can trim it by removing points from the front of the deque.
        while (points.size() > maxPathPoints)
        {
            points.erase(points.begin());
        }

        repaint();
    }

    static constexpr int maxPathPoints = 1000;
    LorenzAudioProcessor& audioProcessor;
    std::vector<juce::Point<float>> points;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttractorComponent)
};
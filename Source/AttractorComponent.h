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
        pointsXZ.reserve(maxPathPoints);
        pointsXY.reserve(maxPathPoints);
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

        // --- Draw Tick Marks ---
        const auto bounds = getLocalBounds().toFloat();

        // Get zoom values from parameters to correctly map tick positions.
        const float viewWidth = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_X");
        const float viewHeightZ = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Z");
        const float viewHeightY = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Y");

        // Define the center points for the view, matching the timerCallback.
        const float centerX = 0.0f;
        const float centerY = 0.0f;
        const float centerZ = 25.0f;

        // Define the mapping ranges based on zoom and center.
        const float lorenzXMin = centerX - viewWidth / 2.0f;
        const float lorenzXMax = centerX + viewWidth / 2.0f;
        const float lorenzYMin = centerY - viewHeightY / 2.0f;
        const float lorenzYMax = centerY + viewHeightY / 2.0f;
        const float lorenzZMin = centerZ - viewHeightZ / 2.0f;
        const float lorenzZMax = centerZ + viewHeightZ / 2.0f;

        // These scaling factors must match the ones in PluginProcessor.cpp
        // to correctly represent the [-1, 1] audio signal range.
        const float xScale = 0.05f;
        const float yScale = 0.05f;
        const float zScale = 0.025f;

        // Calculate the raw coordinate values that correspond to an audio signal of +/- 1.0
        const float xTickValue = 1.0f / xScale; // e.g., 1.0 / 0.05 = 20.0
        const float yTickValue = 1.0f / yScale; // e.g., 1.0 / 0.05 = 20.0
        const float zTickValue = 1.0f / zScale; // e.g., 1.0 / 0.025 = 40.0

        // X-axis ticks (vertical lines)
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        float screenX_neg1 = juce::jmap(-xTickValue, lorenzXMin, lorenzXMax, bounds.getX(), bounds.getRight());
        float screenX_pos1 = juce::jmap( xTickValue, lorenzXMin, lorenzXMax, bounds.getX(), bounds.getRight());
        g.drawVerticalLine(juce::roundToInt(screenX_neg1), bounds.getY(), bounds.getBottom()); // -1 tick
        g.drawVerticalLine(juce::roundToInt(screenX_pos1), bounds.getY(), bounds.getBottom()); // +1 tick

        // Y-axis ticks for X-Y plot (horizontal lines)
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        float screenY_neg1 = juce::jmap(-yTickValue, lorenzYMin, lorenzYMax, bounds.getBottom(), bounds.getY());
        float screenY_pos1 = juce::jmap( yTickValue, lorenzYMin, lorenzYMax, bounds.getBottom(), bounds.getY());
        g.drawHorizontalLine(juce::roundToInt(screenY_neg1), bounds.getX(), bounds.getRight()); // -1 tick
        g.drawHorizontalLine(juce::roundToInt(screenY_pos1), bounds.getX(), bounds.getRight()); // +1 tick

        // Z-axis ticks for X-Z plot (horizontal lines)
        g.setColour(juce::Colours::cyan.withAlpha(0.5f));
        float screenZ_neg1 = juce::jmap(centerZ - zTickValue, lorenzZMin, lorenzZMax, bounds.getBottom(), bounds.getY());
        float screenZ_pos1 = juce::jmap(centerZ + zTickValue, lorenzZMin, lorenzZMax, bounds.getBottom(), bounds.getY());
        g.drawHorizontalLine(juce::roundToInt(screenZ_neg1), bounds.getX(), bounds.getRight()); // -1 tick
        g.drawHorizontalLine(juce::roundToInt(screenZ_pos1), bounds.getX(), bounds.getRight()); // +1 tick

        // --- Draw Attractor Paths ---

        // Create and draw the X-Z path
        juce::Path pathXZ;
        if (! pointsXZ.empty())
        {
            pathXZ.startNewSubPath (pointsXZ.front());
            for (size_t i = 1; i < pointsXZ.size(); ++i)
            {
                pathXZ.lineTo (pointsXZ[i]);
            }
        }
        g.setColour (juce::Colours::white.withAlpha(0.7f));
        g.strokePath (pathXZ, juce::PathStrokeType(1.5f));

        // Create and draw the X-Y path
        juce::Path pathXY;
        if (! pointsXY.empty())
        {
            pathXY.startNewSubPath (pointsXY.front());
            for (size_t i = 1; i < pointsXY.size(); ++i)
            {
                pathXY.lineTo (pointsXY[i]);
            }
        }
        g.setColour (juce::Colours::cyan.withAlpha(0.6f));
        g.strokePath (pathXY, juce::PathStrokeType(1.5f));

        // Draw a border
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        // When the component is resized, we should recalculate the path
        // with the new dimensions. We can clear the points to force a redraw.
        pointsXZ.clear();
        pointsXY.clear();
    }

private:
    void timerCallback() override
    {
        const int maxPointsPerTimerCall = 200;
        const auto bounds = getLocalBounds().toFloat();

        // Get zoom values from parameters. These represent the width/height of the view.
        const float viewWidth = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_X");
        const float viewHeightZ = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Z");
        const float viewHeightY = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Y");

        // For now, we'll keep the view centered.
        const float centerX = 0.0f;
        const float centerY = 0.0f;
        const float centerZ = 25.0f;

        const float lorenzXMin = centerX - viewWidth / 2.0f;
        const float lorenzXMax = centerX + viewWidth / 2.0f;

        const float lorenzYMin = centerY - viewHeightY / 2.0f;
        const float lorenzYMax = centerY + viewHeightY / 2.0f;

        const float lorenzZMin = centerZ - viewHeightZ / 2.0f;
        const float lorenzZMax = centerZ + viewHeightZ / 2.0f;

        for (int i = 0; i < maxPointsPerTimerCall; ++i)
        {
            LorenzAudioProcessor::Point p;
            if (audioProcessor.getPointFromFifo(p))
            {
                // Map Lorenz X coordinate to component's X-axis.
                const float screenX = juce::jmap (p.x, lorenzXMin, lorenzXMax, bounds.getX(), bounds.getRight());
                
                // Map Lorenz Z coordinate to component's Y-axis for the X-Z plot.
                // We map the Lorenz Z range to the full height, and invert the Y-axis so higher Z is higher on screen.
                const float screenZ = juce::jmap (p.z, lorenzZMin, lorenzZMax, bounds.getBottom(), bounds.getY());
                pointsXZ.push_back({screenX, screenZ});
                
                // Map Lorenz Y coordinate to component's Y-axis for the X-Y plot.
                const float screenY = juce::jmap (p.y, lorenzYMin, lorenzYMax, bounds.getBottom(), bounds.getY());
                pointsXY.push_back({screenX, screenY});
            }
            else
            {
                // No more points in the FIFO for now
                break;
            }
        }
        
        // To prevent the path from growing indefinitely and consuming too much memory,
        // we can trim it by removing points from the front of the deque.
        while (pointsXZ.size() > maxPathPoints)
        {
            pointsXZ.erase(pointsXZ.begin());
        }

        while (pointsXY.size() > maxPathPoints)
        {
            pointsXY.erase(pointsXY.begin());
        }

        repaint();
    }

    static constexpr int maxPathPoints = 1000;
    LorenzAudioProcessor& audioProcessor;
    std::vector<juce::Point<float>> pointsXZ;
    std::vector<juce::Point<float>> pointsXY;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttractorComponent)
};
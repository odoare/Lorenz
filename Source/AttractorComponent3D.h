/*
  ==============================================================================

    AttractorComponent3D.h
    Created: 1 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_core/juce_core.h> // Ensure Vector3D and Matrix3D operators are included
#include "PluginProcessor.h"

/**
 * A component to visualize the Lorenz attractor's path.
 * It reads 3D points from a shared FIFO and draws an interactive 3D projection of the path.
*/
class AttractorComponent3D  : public juce::Component, private juce::Timer
{
private:
    // Helper function to apply a 4x4 matrix transform to a 3D vector
    juce::Vector3D<float> applyTransform(const juce::Vector3D<float>& vector, const juce::Matrix3D<float>& matrix)
    {
        // Represent the 3D vector in homogeneous coordinates (x, y, z, 1)
        const float x = vector.x;
        const float y = vector.y;
        const float z = vector.z;
        const float w = 1.0f; // The 'w' component for a point is 1

        // Perform the vector-matrix multiplication: [x, y, z, w] * M
        // JUCE's Matrix3D stores values in row-major order.
        // The multiplication for a row-vector [x, y, z, w] and a row-major matrix M is v' = v * M,
        // which means taking the dot product of the vector with each column of the matrix.
        // x' = x*M(0,0) + y*M(1,0) + z*M(2,0) + w*M(3,0)  (1st column of M)
        // y' = x*M(0,1) + y*M(1,1) + z*M(2,1) + w*M(3,1)  (2nd column of M)
        // etc.

        const float newX = x * matrix.mat[0] + y * matrix.mat[4] + z * matrix.mat[8]  + w * matrix.mat[12];
        const float newY = x * matrix.mat[1] + y * matrix.mat[5] + z * matrix.mat[9]  + w * matrix.mat[13];
        const float newZ = x * matrix.mat[2] + y * matrix.mat[6] + z * matrix.mat[10] + w * matrix.mat[14];
        const float newW = x * matrix.mat[3] + y * matrix.mat[7] + z * matrix.mat[11] + w * matrix.mat[15];

        // Convert back from homogeneous to Cartesian coordinates by dividing by w'
        // For rotations, w' will be 1, but this is the general and correct way.
        if (newW != 0.0f)
        {
            return { newX / newW, newY / newW, newZ / newW };
        }

        // In the unlikely event w' is zero, return the original vector
        return vector;
    }

public:
    AttractorComponent3D(LorenzAudioProcessor& p) : audioProcessor(p)
    {
        pointsXYZ.reserve(maxPathPoints);
        // Start the timer to update the display. 50Hz is a good rate for smooth animation

        addAndMakeVisible(xyViewButton);
        xyViewButton.setButtonText("XY");
        xyViewButton.onClick = [this] { rotationX = 0.0f; rotationY = 0.0f; repaint(); };

        addAndMakeVisible(xzViewButton);
        xzViewButton.setButtonText("XZ");
        xzViewButton.onClick = [this] { rotationX = juce::MathConstants<float>::pi / 2.0f; rotationY = 0.0f; repaint(); };

        addAndMakeVisible(yzViewButton);
        yzViewButton.setButtonText("YZ");
        yzViewButton.onClick = [this] { rotationX = juce::MathConstants<float>::pi / 2.0f; rotationY = juce::MathConstants<float>::pi / 2.0f; repaint(); };

        addAndMakeVisible(isoViewButton);
        isoViewButton.setButtonText("3D");
        isoViewButton.onClick = [this] { rotationX = 0.615f; rotationY = -0.785f; repaint(); }; // ~35.26 deg, -45 deg

        xyViewButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.5f));
        xzViewButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.5f));
        yzViewButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.5f));
        isoViewButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.5f));

        startTimerHz(50);
    }

    ~AttractorComponent3D() override
    {
        stopTimer();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        const auto bounds = getLocalBounds().toFloat();
        const float componentWidth = bounds.getWidth();
        const float componentHeight = bounds.getHeight();

        // Get zoom values from parameters.
        const float viewZoomX = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_X");
        const float viewZoomY = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Y");
        const float viewZoomZ = *audioProcessor.apvts.getRawParameterValue("VIEW_ZOOM_Z");

        // Define the center of the Lorenz attractor in its own coordinate space
        const juce::Vector3D<float> lorenzCenter (0.0f, 0.0f, 25.0f);

        // Create rotation matrices
        // We will construct the matrices manually to ensure compatibility.
        const float cosX = std::cos(rotationX);
        const float sinX = std::sin(rotationX);
        const juce::Matrix3D<float> rotX (1.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f, cosX, -sinX, 0.0f,
                                          0.0f, sinX,  cosX, 0.0f,
                                          0.0f, 0.0f,  0.0f, 1.0f);

        const float cosY = std::cos(rotationY);
        const float sinY = std::sin(rotationY);
        const juce::Matrix3D<float> rotY ( cosY, 0.0f, sinY, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                          -sinY, 0.0f, cosY, 0.0f,
                                           0.0f, 0.0f, 0.0f, 1.0f);

        const auto rotationMatrix = rotX * rotY;


        juce::Path path;
        if (pointsXYZ.size() < 2)
            return;

        // Project the first point to start the path
        juce::Vector3D<float> p0 = pointsXYZ.front();
        p0 -= lorenzCenter; // Center the point
        p0.x /= viewZoomX;  // Apply zoom
        p0.y /= viewZoomY;
        p0.z /= viewZoomZ;
        p0 = applyTransform(p0, rotationMatrix); // Apply rotation

        // Simple perspective projection
        const float perspective = 1.0f + p0.z;
        const float projectedX0 = componentWidth * (0.5f + p0.x * perspective);
        const float projectedY0 = componentHeight * (0.5f - p0.y * perspective);

        path.startNewSubPath(projectedX0, projectedY0);

        // Create the rest of the path with variable brightness and thickness for depth
        for (size_t i = 1; i < pointsXYZ.size(); ++i)
        {
            juce::Vector3D<float> p1 = pointsXYZ[i];
            p1 -= lorenzCenter;
            p1.x /= viewZoomX;
            p1.y /= viewZoomY;
            p1.z /= viewZoomZ;
            p1 = applyTransform(p1, rotationMatrix); // Apply rotation

            const float perspective1 = 1.0f + p1.z;
            const float projectedX1 = componentWidth * (0.5f + p1.x * perspective1);
            const float projectedY1 = componentHeight * (0.5f - p1.y * perspective1);

            // Use the Z-coordinate to simulate depth (brighter and thicker when closer)
            const float brightness = juce::jmap(p1.z, -0.5f, 0.5f, 0.4f, 1.0f);
            const float thickness = juce::jmap(p1.z, -0.5f, 0.5f, 1.0f, 2.5f);

            g.setColour(juce::Colours::cyan.brighter().withAlpha(brightness));
            g.drawLine(path.getCurrentPosition().x, path.getCurrentPosition().y, projectedX1, projectedY1, thickness);
            path.lineTo(projectedX1, projectedY1);
        }

        // --- Draw Axes in bottom-left corner ---
        {
            const float axisLength = 0.5f; // Relative length of axis lines
            const float axisLabelOffset = 1.2f; // How far from the axis end to draw the label
            const juce::Vector3D<float> origin(0.0f, 0.0f, 0.0f);
            const juce::Vector3D<float> xAxis(axisLength, 0.0f, 0.0f);
            const juce::Vector3D<float> yAxis(0.0f, axisLength, 0.0f);
            const juce::Vector3D<float> zAxis(0.0f, 0.0f, axisLength);

            // Rotate the axes along with the model
            const auto tOrigin = applyTransform(origin, rotationMatrix);
            const auto tX = applyTransform(xAxis, rotationMatrix);
            const auto tY = applyTransform(yAxis, rotationMatrix);
            const auto tZ = applyTransform(zAxis, rotationMatrix);

            // Define a helper lambda for projecting and positioning the axes
            auto projectAxisPoint = [&](const juce::Vector3D<float>& p)
            {
                // We use a fixed perspective and scale for the gizmo
                const float p_persp = 1.0f + p.z * 0.5f;
                const float p_x = 50.0f + p.x * 50.0f * p_persp; // Position in bottom-left
                const float p_y = componentHeight - 50.0f - p.y * 50.0f * p_persp;
                return juce::Point<float>(p_x, p_y);
            };

            const auto pOrigin = projectAxisPoint(tOrigin);
            const auto pX = projectAxisPoint(tX);
            const auto pY = projectAxisPoint(tY);
            const auto pZ = projectAxisPoint(tZ);

            g.setColour(juce::Colours::red);   g.drawLine({pOrigin, pX}, 2.0f); g.drawText("X", pX.x + (pX.x-pOrigin.x)*0.1f - 5, pX.y + (pX.y-pOrigin.y)*0.1f - 7, 10, 14, juce::Justification::centred);
            g.setColour(juce::Colours::green); g.drawLine({pOrigin, pY}, 2.0f); g.drawText("Y", pY.x + (pY.x-pOrigin.x)*0.1f - 5, pY.y + (pY.y-pOrigin.y)*0.1f - 7, 10, 14, juce::Justification::centred);
            g.setColour(juce::Colours::blue);  g.drawLine({pOrigin, pZ}, 2.0f); g.drawText("Z", pZ.x + (pZ.x-pOrigin.x)*0.1f - 5, pZ.y + (pZ.y-pOrigin.y)*0.1f - 7, 10, 14, juce::Justification::centred);
        }

        // Draw a border
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        // When the component is resized, we should clear the points
        // to force a full redraw in the new aspect ratio.
        pointsXYZ.clear();

        const int buttonWidth = 40;
        const int buttonHeight = 20;
        const int margin = 4;
        xyViewButton.setBounds(margin, getHeight() - buttonHeight - margin, buttonWidth, buttonHeight);
        xzViewButton.setBounds(margin + buttonWidth, getHeight() - buttonHeight - margin, buttonWidth, buttonHeight);
        yzViewButton.setBounds(margin + 2 * buttonWidth, getHeight() - buttonHeight - margin, buttonWidth, buttonHeight);
        isoViewButton.setBounds(margin + 3 * buttonWidth, getHeight() - buttonHeight - margin, buttonWidth, buttonHeight);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        // Use the distance dragged to update the rotation angles.
        // The sensitivity can be adjusted by changing the divisor.
        rotationY += (float) event.getOffsetFromDragStart().x / 200.0f;
        rotationX += (float) event.getOffsetFromDragStart().y / 200.0f;
        repaint();
    }

private:
    void timerCallback() override
    {
        const int maxPointsPerTimerCall = 200;
        bool needsRepaint = false;

        for (int i = 0; i < maxPointsPerTimerCall; ++i)
        {
            LorenzAudioProcessor::Point p;
            if (audioProcessor.getPointFromFifo(p))
            {
                pointsXYZ.push_back({p.x, p.y, p.z});
                needsRepaint = true;
            }
            else
            {
                break; // FIFO is empty
            }
        }

        while (pointsXYZ.size() > maxPathPoints)
        {
            pointsXYZ.erase(pointsXYZ.begin());
        }

        if (needsRepaint)
            repaint();
    }

    static constexpr int maxPathPoints = 1000;
    LorenzAudioProcessor& audioProcessor;
    std::vector<juce::Vector3D<float>> pointsXYZ;

    float rotationX = 0.3f;
    float rotationY = 0.0f;

    juce::TextButton xyViewButton;
    juce::TextButton xzViewButton;
    juce::TextButton yzViewButton;
    juce::TextButton isoViewButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttractorComponent3D)
};
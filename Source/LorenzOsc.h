/*
  ==============================================================================

    LorenzOsc.h
    Created: 1 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * Implements a Lorenz attractor oscillator.
 * It uses numerical integration to solve the Lorenz system of differential equations
 * and uses one of the state variables as an audio output.
*/
class LorenzOsc
{
public:
    LorenzOsc();

    void prepareToPlay(double sampleRate);
    std::tuple<double, double, double> getNextSample();

    void reset();
    void setParameters(const std::atomic<float>* newAlpha, const std::atomic<float>* newBeta, const std::atomic<float>* newGamma,
                       const std::atomic<float>* newDelta, const std::atomic<float>* newFrequency);
    void setTimestep(const std::atomic<float>* newDt);
    void setRampLength(double rampLengthSeconds);

private:
    // Duffing system state
    double x, v, t;

    // Duffing system parameters
    juce::SmoothedValue<float> alpha, beta, gamma, delta, frequency;

    // Timestep for numerical integration
    juce::SmoothedValue<float> dt;

    // Raw pointers to APVTS parameters
    const std::atomic<float>* alphaParam { nullptr }, *betaDuffingParam { nullptr }, *gammaParam { nullptr }, *deltaParam { nullptr }, *frequencyParam { nullptr };
    const std::atomic<float>* dtParam { nullptr };

    // Sample rate
    double sampleRate;
    double rampDurationSeconds;
};
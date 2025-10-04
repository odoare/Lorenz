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

    void setParameters(const std::atomic<float>* newSigma, const std::atomic<float>* newRho, const std::atomic<float>* newBeta);
    void setTimestep(const std::atomic<float>* newTimestep);

private:
    // Lorenz system state
    double x, y, z, vx;

    // Lorenz system parameters
    const std::atomic<float>* sigma { nullptr };
    const std::atomic<float>* rho   { nullptr };
    const std::atomic<float>* beta  { nullptr };

    // Timestep for numerical integration
    const std::atomic<float>* dt { nullptr };

    // Sample rate
    double sampleRate;
};
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
    void setParameters(const std::atomic<float>* newSigma, const std::atomic<float>* newRho, const std::atomic<float>* newBeta,
                       const std::atomic<float>* newMx, const std::atomic<float>* newMy, const std::atomic<float>* newMz, //
                       const std::atomic<float>* newCx, const std::atomic<float>* newCy, const std::atomic<float>* newCz, //
                       const std::atomic<float>* newTaming); //
    void setTimestep(const std::atomic<float>* newDt);
    void updateParameters();
    void setRampLength(double rampLengthSeconds);

private:
    // Lorenz system state
    double x, y, z, vx, vy, vz;

    // Lorenz system parameters
    juce::SmoothedValue<float> sigma, rho, beta;

    // Second order parameters
    juce::SmoothedValue<float> mx, my, mz;
    juce::SmoothedValue<float> cx, cy, cz;

    // Taming parameter
    juce::SmoothedValue<float> taming;

    // Timestep for numerical integration
    juce::SmoothedValue<float> dt;

    // Raw pointers to APVTS parameters
    const std::atomic<float>* sigmaParam { nullptr }, *rhoParam { nullptr }, *betaParam { nullptr };
    const std::atomic<float>* mxParam { nullptr }, *myParam { nullptr }, *mzParam { nullptr };
    const std::atomic<float>* cxParam { nullptr }, *cyParam { nullptr }, *czParam { nullptr };
    const std::atomic<float>* dtParam { nullptr };
    const std::atomic<float>* tamingParam { nullptr };

    // Sample rate
    double sampleRate;
    double rampDurationSeconds;
};
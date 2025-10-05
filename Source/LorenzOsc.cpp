/*
  ==============================================================================

    LorenzOsc.cpp
    Created: 1 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#include "LorenzOsc.h"

LorenzOsc::LorenzOsc()
{
    reset();
    sampleRate = 44100.0;
    rampDurationSeconds = 0.05; // 50ms default ramp time
}

void LorenzOsc::prepareToPlay(double sr)
{
    sampleRate = sr;
    alpha.reset(sampleRate, rampDurationSeconds);
    beta.reset(sampleRate, rampDurationSeconds);
    gamma.reset(sampleRate, rampDurationSeconds);
    delta.reset(sampleRate, rampDurationSeconds);
    frequency.reset(sampleRate, rampDurationSeconds);
    dt.reset(sampleRate, rampDurationSeconds);
}

void LorenzOsc::reset()
{
    // Initial state
    x = 0.1; // Start with a small non-zero value
    v = 0.0;
    t = 0.0;
}

void LorenzOsc::setParameters(const std::atomic<float>* newAlpha, const std::atomic<float>* newBeta, const std::atomic<float>* newGamma,
                              const std::atomic<float>* newDelta, const std::atomic<float>* newFrequency)
{
    alphaParam = newAlpha;
    betaDuffingParam = newBeta;
    gammaParam = newGamma;
    deltaParam = newDelta;
    frequencyParam = newFrequency;
}

void LorenzOsc::setTimestep(const std::atomic<float>* newDt)
{
    dtParam = newDt;
}
void LorenzOsc::setRampLength(double rampLengthSeconds)
{
    this->rampDurationSeconds = rampLengthSeconds;
    alpha.reset(sampleRate, this->rampDurationSeconds);
    beta.reset(sampleRate, this->rampDurationSeconds);
    gamma.reset(sampleRate, this->rampDurationSeconds);
    delta.reset(sampleRate, this->rampDurationSeconds);
    frequency.reset(sampleRate, this->rampDurationSeconds);
    dt.reset(sampleRate, this->rampDurationSeconds);
}

std::tuple<double, double, double> LorenzOsc::getNextSample()
{
    // Set the target for the smoothed values from the parameters
    alpha.setTargetValue(alphaParam->load());
    beta.setTargetValue(betaDuffingParam->load());
    gamma.setTargetValue(gammaParam->load());
    delta.setTargetValue(deltaParam->load());
    frequency.setTargetValue(frequencyParam->load());
    dt.setTargetValue(dtParam->load());
    
    // --- Fourth-Order Runge-Kutta (RK4) Integration ---

    // Helper lambda to compute derivatives at a given state
    // We pass the parameters by value to ensure the same values are used for all RK4 steps.
    auto derivatives = [&](double current_t, double current_x, double current_v,
                           float p_alpha, float p_beta, float p_gamma, float p_delta, float p_omega) -> std::tuple<double, double>
    {
        const double dxdt = current_v;
        const double dvdt = p_gamma * std::cos(p_omega * current_t) - p_delta * current_v - p_alpha * current_x - p_beta * (current_x * current_x * current_x);
        
        return { dxdt, dvdt };
    };

    const float currentDt = dt.getNextValue();
    const float currentAlpha = alpha.getNextValue();
    const float currentBeta = beta.getNextValue();
    const float currentGamma = gamma.getNextValue();
    const float currentDelta = delta.getNextValue();
    const float currentOmega = juce::MathConstants<float>::twoPi * frequency.getNextValue();

    // k1: Evaluate derivatives at the current state
    auto [k1_x, k1_v] = derivatives(t, x, v, currentAlpha, currentBeta, currentGamma, currentDelta, currentOmega);

    // k2: Evaluate at midpoint using k1
    auto [k2_x, k2_v] = derivatives(t + 0.5 * currentDt, x + 0.5 * currentDt * k1_x, v + 0.5 * currentDt * k1_v, currentAlpha, currentBeta, currentGamma, currentDelta, currentOmega);

    // k3: Evaluate at midpoint using k2
    auto [k3_x, k3_v] = derivatives(t + 0.5 * currentDt, x + 0.5 * currentDt * k2_x, v + 0.5 * currentDt * k2_v, currentAlpha, currentBeta, currentGamma, currentDelta, currentOmega);

    // k4: Evaluate at the end of the step using k3
    auto [k4_x, k4_v] = derivatives(t + currentDt, x + currentDt * k3_x, v + currentDt * k3_v, currentAlpha, currentBeta, currentGamma, currentDelta, currentOmega);

    // Update state using the weighted average of the k-values
    x += (currentDt / 6.0) * (k1_x + 2.0 * k2_x + 2.0 * k3_x + k4_x);
    v += (currentDt / 6.0) * (k1_v + 2.0 * k2_v + 2.0 * k3_v + k4_v);
    
    // Increment time for the next sample's forcing term calculation. This is crucial!
    t += currentDt;

    // --- Stability Check ---
    // If any state variable becomes non-finite, reset the system.
    if (! (std::isfinite(x) && std::isfinite(v)))
    {
        reset();
    }

    // The raw values are returned. Scaling will be handled by the processor.
    // We'll output x for the audio, and v for the visualization's z-axis.
    return { x, v, 0.0 };
}
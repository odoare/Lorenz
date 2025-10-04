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
    setRampLength(rampDurationSeconds);
}

void LorenzOsc::prepareToPlay(double sr)
{
    sampleRate = sr;
    sigma.reset(sampleRate, rampDurationSeconds);
    rho.reset(sampleRate, rampDurationSeconds);
    beta.reset(sampleRate, rampDurationSeconds);
    mx.reset(sampleRate, rampDurationSeconds);
    my.reset(sampleRate, rampDurationSeconds);
    mz.reset(sampleRate, rampDurationSeconds);
    cx.reset(sampleRate, rampDurationSeconds);
    cy.reset(sampleRate, rampDurationSeconds);
    cz.reset(sampleRate, rampDurationSeconds);
    dt.reset(sampleRate, rampDurationSeconds);
}

void LorenzOsc::reset()
{
    // Initial state
    x = 0.1;
    y = 0.0;
    z = 0.0;
    vx = 0.0;
    vy = 0.0;
    vz = 0.0;
}

void LorenzOsc::setParameters(const std::atomic<float>* newSigma, const std::atomic<float>* newRho, const std::atomic<float>* newBeta,
                              const std::atomic<float>* newMx, const std::atomic<float>* newMy, const std::atomic<float>* newMz,
                              const std::atomic<float>* newCx, const std::atomic<float>* newCy, const std::atomic<float>* newCz)
{
    sigmaParam = newSigma;
    rhoParam = newRho;
    betaParam = newBeta;
    mxParam = newMx;
    myParam = newMy;
    mzParam = newMz;
    cxParam = newCx;
    cyParam = newCy;
    czParam = newCz;
}

void LorenzOsc::setTimestep(const std::atomic<float>* newDt)
{
    dtParam = newDt;
}

void LorenzOsc::setRampLength(double rampLengthSeconds)
{
    this->rampDurationSeconds = rampLengthSeconds;
    sigma.reset(sampleRate, this->rampDurationSeconds);
    rho.reset(sampleRate, this->rampDurationSeconds);
    beta.reset(sampleRate, this->rampDurationSeconds);
    mx.reset(sampleRate, this->rampDurationSeconds);
    my.reset(sampleRate, this->rampDurationSeconds);
    mz.reset(sampleRate, this->rampDurationSeconds);
    cx.reset(sampleRate, this->rampDurationSeconds);
    cy.reset(sampleRate, this->rampDurationSeconds);
    cz.reset(sampleRate, this->rampDurationSeconds);
    dt.reset(sampleRate, this->rampDurationSeconds);
}

std::tuple<double, double, double> LorenzOsc::getNextSample()
{
    // Set the target for the smoothed values from the parameters
    sigma.setTargetValue(sigmaParam->load());
    rho.setTargetValue(rhoParam->load());
    beta.setTargetValue(betaParam->load());
    mx.setTargetValue(mxParam->load());
    my.setTargetValue(myParam->load());
    mz.setTargetValue(mzParam->load());
    cx.setTargetValue(cxParam->load());
    cy.setTargetValue(cyParam->load());
    cz.setTargetValue(czParam->load());
    dt.setTargetValue(dtParam->load());
    
    // --- Fourth-Order Runge-Kutta (RK4) Integration ---

    // Helper lambda to compute derivatives at a given state
    auto derivatives = [&](double tX, double tY, double tZ, double tVx, double tVy, double tVz) -> std::tuple<double, double, double, double, double, double>
    {
        const double dxdt = tVx;
        const double dydt = tVy;
        const double dzdt = tVz;

        // Get the next smoothed value for this sample
        const float currentSigma = sigma.getNextValue();
        const float currentRho = rho.getNextValue();
        const float currentBeta = beta.getNextValue();
        const float currentMx = mx.getNextValue();
        const float currentMy = my.getNextValue();
        const float currentMz = mz.getNextValue();
        const float currentCx = cx.getNextValue();
        const float currentCy = cy.getNextValue();
        const float currentCz = cz.getNextValue();

        const double dvxdt = (currentSigma * (tY - tX) - currentCx * tVx) / currentMx;
        const double dvydt = (tX * (currentRho - tZ) - tY - currentCy * tVy) / currentMy;
        const double dvzdt = (tX * tY - currentBeta * tZ - currentCz * tVz) / currentMz;

        return { dxdt, dydt, dzdt, dvxdt, dvydt, dvzdt };
    };

    const float currentDt = dt.getNextValue();

    // k1: Evaluate derivatives at the current state
    auto [k1_x, k1_y, k1_z, k1_vx, k1_vy, k1_vz] = derivatives(x, y, z, vx, vy, vz);

    // k2: Evaluate at midpoint using k1
    auto [k2_x, k2_y, k2_z, k2_vx, k2_vy, k2_vz] = derivatives(x + 0.5 * currentDt * k1_x,
                                                               y + 0.5 * currentDt * k1_y,
                                                               z + 0.5 * currentDt * k1_z,
                                                               vx + 0.5 * currentDt * k1_vx,
                                                               vy + 0.5 * currentDt * k1_vy,
                                                               vz + 0.5 * currentDt * k1_vz);

    // k3: Evaluate at midpoint using k2
    auto [k3_x, k3_y, k3_z, k3_vx, k3_vy, k3_vz] = derivatives(x + 0.5 * currentDt * k2_x,
                                                               y + 0.5 * currentDt * k2_y,
                                                               z + 0.5 * currentDt * k2_z,
                                                               vx + 0.5 * currentDt * k2_vx,
                                                               vy + 0.5 * currentDt * k2_vy,
                                                               vz + 0.5 * currentDt * k2_vz);

    // k4: Evaluate at the end of the step using k3
    auto [k4_x, k4_y, k4_z, k4_vx, k4_vy, k4_vz] = derivatives(x + currentDt * k3_x,
                                                               y + currentDt * k3_y,
                                                               z + currentDt * k3_z,
                                                               vx + currentDt * k3_vx,
                                                               vy + currentDt * k3_vy,
                                                               vz + currentDt * k3_vz);

    // Update state using the weighted average of the k-values
    x += (currentDt / 6.0) * (k1_x + 2.0 * k2_x + 2.0 * k3_x + k4_x);
    y += (currentDt / 6.0) * (k1_y + 2.0 * k2_y + 2.0 * k3_y + k4_y);
    z += (currentDt / 6.0) * (k1_z + 2.0 * k2_z + 2.0 * k3_z + k4_z);
    vx += (currentDt / 6.0) * (k1_vx + 2.0 * k2_vx + 2.0 * k3_vx + k4_vx);
    vy += (currentDt / 6.0) * (k1_vy + 2.0 * k2_vy + 2.0 * k3_vy + k4_vy);
    vz += (currentDt / 6.0) * (k1_vz + 2.0 * k2_vz + 2.0 * k3_vz + k4_vz);

    // --- Stability Check ---
    // If any state variable becomes non-finite, reset the system.
    if (! (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)))
    {
        reset();
    }

    // The raw values are returned. Scaling will be handled by the processor.
    return { x, y, z };
}
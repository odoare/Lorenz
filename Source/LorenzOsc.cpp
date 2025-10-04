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
    // Initial state
    x = 0.1;
    y = 0.0;
    z = 0.0;
    vx = 0.0;

    sampleRate = 44100.0; // Default sample rate
}

void LorenzOsc::prepareToPlay(double sr)
{
    sampleRate = sr;
}

void LorenzOsc::setParameters(const std::atomic<float>* newSigma, const std::atomic<float>* newRho, const std::atomic<float>* newBeta)
{
    sigma = newSigma;
    rho = newRho;
    beta = newBeta;
}

void LorenzOsc::setTimestep(const std::atomic<float>* newTimestep)
{
    dt = newTimestep;
}

std::tuple<double, double, double> LorenzOsc::getNextSample()
{
    // It's important to load the atomic parameter values here, inside the process block.
    const float currentSigma = sigma->load();
    const float currentRho = rho->load();
    const float currentBeta = beta->load();
    const float currentDt = dt->load();
    
    // --- Fourth-Order Runge-Kutta (RK4) Integration ---

    // Helper lambda to compute derivatives at a given state
    auto derivatives = [&](double tempX, double tempY, double tempZ, double tempVx) -> std::tuple<double, double, double, double>
    {
        const double dxdt = tempVx;
        const double dvxdt = currentSigma * (tempY - tempX);
        const double dydt = tempX * (currentRho - tempZ) - tempY;
        const double dzdt = tempX * tempY - currentBeta * tempZ;
        return { dxdt, dydt, dzdt, dvxdt };
    };

    // k1: Evaluate derivatives at the current state
    auto [k1_x, k1_y, k1_z, k1_vx] = derivatives(x, y, z, vx);

    // k2: Evaluate at midpoint using k1
    auto [k2_x, k2_y, k2_z, k2_vx] = derivatives(x + 0.5 * currentDt * k1_x,
                                                 y + 0.5 * currentDt * k1_y,
                                                 z + 0.5 * currentDt * k1_z,
                                                 vx + 0.5 * currentDt * k1_vx);

    // k3: Evaluate at midpoint using k2
    auto [k3_x, k3_y, k3_z, k3_vx] = derivatives(x + 0.5 * currentDt * k2_x,
                                                 y + 0.5 * currentDt * k2_y,
                                                 z + 0.5 * currentDt * k2_z,
                                                 vx + 0.5 * currentDt * k2_vx);

    // k4: Evaluate at the end of the step using k3
    auto [k4_x, k4_y, k4_z, k4_vx] = derivatives(x + currentDt * k3_x,
                                                 y + currentDt * k3_y,
                                                 z + currentDt * k3_z,
                                                 vx + currentDt * k3_vx);

    // Update state using the weighted average of the k-values
    x += (currentDt / 6.0) * (k1_x + 2.0 * k2_x + 2.0 * k3_x + k4_x);
    y += (currentDt / 6.0) * (k1_y + 2.0 * k2_y + 2.0 * k3_y + k4_y);
    z += (currentDt / 6.0) * (k1_z + 2.0 * k2_z + 2.0 * k3_z + k4_z);
    vx += (currentDt / 6.0) * (k1_vx + 2.0 * k2_vx + 2.0 * k3_vx + k4_vx);

    // The raw values are returned. Scaling will be handled by the processor.
    return { x, y, z };
}
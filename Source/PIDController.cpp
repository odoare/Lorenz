/*
  ==============================================================================

    PIDController.cpp
    Created: 2 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#include "PIDController.h"

void PIDController::setGains(float proportional, float integral, float derivative)
{
    kp = proportional;
    ki = integral;
    kd = derivative;
}

void PIDController::setIntegralLimits(float min, float max)
{
    minIntegral = min;
    maxIntegral = max;
}

float PIDController::process(float targetValue, float currentValue)
{
    const float error = targetValue - currentValue;

    // Proportional term
    const float proportionalTerm = error * kp;

    // Integral term with anti-windup
    integral += error * ki;
    integral = juce::jlimit(minIntegral, maxIntegral, integral);

    // Derivative term
    const float derivativeTerm = (error - lastError) * kd;
    lastError = error;

    return proportionalTerm + integral + derivativeTerm;
}

void PIDController::reset()
{
    integral = 0.0f;
    lastError = 0.0f;
}
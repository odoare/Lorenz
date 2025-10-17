/*
  ==============================================================================

    PIDController.h
    Created: 2 Oct 2025 10:00:00am
    Author:  Olivier Doaré

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * A generic PID (Proportional-Integral-Derivative) controller.
 */
class PIDController
{
public:
    PIDController() = default;

    /** Sets the gain parameters for the controller. */
    void setGains(float proportional, float integral, float derivative);

    /** Sets the limits for the integral term to prevent wind-up. */
    void setIntegralLimits(float min, float max);

    /**
     * Calculates the control output.
     * @param targetValue  The desired value (setpoint).
     * @param currentValue The measured value from the process.
     * @return The calculated control signal adjustment.
     */
    float process(float targetValue, float currentValue);

    /** Resets the controller's internal state (integral and derivative terms). */
    void reset();

private:
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    float integral = 0.0f;
    float lastError = 0.0f;
    float minIntegral = -1.0f;
    float maxIntegral = 1.0f;
};
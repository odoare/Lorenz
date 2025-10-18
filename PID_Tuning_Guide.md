During my experiments I found that a good standard stating point for PID gains are the following:

- KD= 2e-7
- KI = 6e-8
- KP = 4.7-6

with a dt=0.01s as the timescale of the PID.

Below is a guide I asked Gemini to write for the tuning of this synthesis.

# A Practical Guide to Tuning the PID Controller

Tuning the PID controller for the Lorenz plugin is a classic engineering task that's as much an art as it is a science, especially in a creative context like this one. The goal is to find the right balance between a fast response, minimal overshoot, and stability.

## Understanding the Role of Each Gain

First, let's understand what each parameter does in the context of your plugin. The "error" is the difference between the `targetFrequency` (the note you played) and the `measuredFrequency` (what the plugin is currently producing).

### Kp (Proportional Gain - The "Driver")

*   **What it does:** Reacts to the *current* error. The larger the error, the stronger the correction.
*   **Effect:** This is the primary force that pushes the pitch towards the target. A higher `Kp` makes the system react faster.
*   **Too low:** The pitch will be sluggish and may never reach the target.
*   **Too high:** The pitch will overshoot the target and oscillate wildly, like a car with overly sensitive steering.

### Ki (Integral Gain - The "Corrector")

*   **What it does:** Reacts to the *accumulation of past errors*. It sums up the error over time.
*   **Effect:** This term works to eliminate small, persistent "steady-state" errors. If your pitch is consistently 1Hz flat, the integral term will slowly build up and push the pitch to be perfectly in tune.
*   **Too low:** Small errors might persist, and the pitch may never lock on perfectly.
*   **Too high:** It can cause "integral wind-up," leading to massive overshoots and slow, long-period oscillations. It can easily de-stabilize the system.

### Kd (Derivative Gain - The "Damper")

*   **What it does:** Reacts to the *rate of change* of the error. It essentially predicts future error and acts to counteract it.
*   **Effect:** This term acts as a brake or damper, reducing the overshoot and oscillations caused by `Kp`. It helps the system settle down quickly and smoothly.
*   **Too low:** The pitch will overshoot and "ring" around the target frequency.
*   **Too high:** The system will become overly sensitive to noise in the pitch measurement, leading to jittery, nervous behavior. It can also make the response sluggish if it's damping too much.

## A Practical Step-by-Step Tuning Method

The most effective way to tune is manually, one parameter at a time, using the GUI knobs.

1.  **Step 1: Set All Gains to Zero**
    *   Turn the `Kp`, `Ki`, and `Kd` knobs all the way down.
    *   Play a note. The pitch will not be controlled and will wander freely.

2.  **Step 2: Tune Proportional Gain (`Kp`)**
    *   Keep `Ki` and `Kd` at zero.
    *   Slowly increase `Kp`. As you do, you'll notice the pitch starts to move towards the note you're playing.
    *   Keep increasing `Kp` until the pitch starts to oscillate consistently around the target note. It won't settle, but it will be actively trying. This oscillation is what you're looking for.
    *   **Goal:** Find the `Kp` value that gives you a fast response. Note the value that causes sustained oscillation. A good starting point for your final `Kp` is often around **50-60% of that value**.

3.  **Step 3: Tune Derivative Gain (`Kd`)**
    *   Keep `Kp` at the value you just found. `Ki` is still zero.
    *   Now, slowly increase `Kd`.
    *   You should see the oscillations from `Kp` start to reduce. The pitch will overshoot less and settle down more quickly.
    *   **Goal:** Find the `Kd` value that effectively dampens the system without making it feel "stiff" or jittery. If the pitch becomes noisy or erratic, your `Kd` is too high and is amplifying noise from the pitch detector.

4.  **Step 4: Tune Integral Gain (`Ki`)**
    *   With your tuned `Kp` and `Kd`, play a note and let it settle. You might notice it's consistently a little sharp or flat. This is the steady-state error.
    *   Slowly increase `Ki`.
    *   You should see this small, persistent error gradually disappear as the integral term builds up and nudges the pitch perfectly into tune.
    *   **Goal:** Use the **smallest possible value of `Ki`** that eliminates the steady-state error. Too much `Ki` is a very common cause of instability and will undo the stability you achieved with `Kd`.

5.  **Step 5: Fine-Tuning**
    *   Now that you have baseline values for all three, play different notes and observe the response.
    *   You may need to iterate. For example, after adding `Ki`, you might need to slightly increase `Kd` again to handle the new overshoot, or slightly decrease `Kp`.
    *   The "perfect" tuning depends on the sound you want. Do you want a loose, "drunken" pitch that wanders around the note? Use lower gains. Do you want a tight, robotic lock? Use higher, more aggressive gains.

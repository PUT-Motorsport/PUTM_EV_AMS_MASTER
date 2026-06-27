#pragma once

namespace PUTM
{
    /**
     * @brief  Second order Infinite Impulse Response (IIR) filter class.
     */
    class BiquadIIR
    {
    private:
        float a0, a1, a2; // Feedforward coefficients
        float b1, b2;     // Feedback coefficients
        float z1, z2;     // State variables
    public:
        /**
         * @brief  Constructor for the BiquadIIR class.
         * @param  a0, a1, a2 Feedforward coefficients.
         * @param  b1, b2 Feedback coefficients.
         */
        BiquadIIR(float a0, float a1, float a2, float b1, float b2)
            : a0(a0), a1(a1), a2(a2), b1(b1), b2(b2), z1(0.0f), z2(0.0f) {}
    public:
        /**
         * @brief  Update the filter with a new input value and get the filtered output.
         * @param  input The new input value to be filtered.
         * @return The filtered output value.
         */
        float update(float input)
        {
            float output = a0 * input + z1;
            z1 = a1 * input - b1 * output + z2;
            z2 = a2 * input - b2 * output;
            return output;
        }
    };
}
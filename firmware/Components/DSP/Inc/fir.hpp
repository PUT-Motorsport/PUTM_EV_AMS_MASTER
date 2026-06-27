#pragma once

namespace PUTM
{
    /**
     *  @brief  A simple FIR filter class for filtering data.
     *  @tparam N The number of taps in the filter.
     *  @tparam TYPE The type of the input and output data.
     *  @note   This class is designed to be used with fixed-size filters.
     *          It does not support dynamic resizing or complex operations.
     */
    template<size_t N, class TYPE = float>
    requires (N > 0) && std::is_arithmetic_v<TYPE>
    class Fir
    {
    private:
        TYPE coeffs[N] { };
        TYPE buffer[N] { };
        size_t index { 0 };
    public:
        using type = TYPE;
    public:
        /**
         *  @brief  Default constructor for the FIR class.
         *  @note   Initializes all coefficients to zero.
         */
        Fir() = default;
    public:
        /**
         *  @brief  Constructor for the FIR class with coefficients.
         *  @param  coeffs An array of coefficients to initialize the filter.
         */
        consteval Fir(const TYPE (&coeffs)[N])
        {
            if consteval
            {
                static_for<0, N>([&]<int I>() 
                {
                    this->coeffs[I] = coeffs[I];
                });
            }
            else
            {
                for (size_t i = 0; i < N; ++i)
                {
                    this->coeffs[i] = coeffs[i];
                }
            }
        }
    public:
        /**
         *  @brief  Update the filter with a new input value and get the filtered output.
         *  @param  input The new input value to be filtered.
         *  @return The filtered output value.
         */
        TYPE update(TYPE input)
        {
            buffer[index] = input;
            index = (index + 1) % N;

            TYPE output { 0 };
            for (size_t i = 0; i < N; ++i)
            {
                size_t buf_index = (index + i) % N;
                output += coeffs[i] * buffer[buf_index];
            }

            return output;
        }
    };
}
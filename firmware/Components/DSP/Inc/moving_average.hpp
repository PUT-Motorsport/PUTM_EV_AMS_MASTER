#pragma once

namespace PUTM
{
    template<size_t N, class TYPE = float>
    requires (N > 0) && std::is_arithmetic_v<TYPE>
    class MovingAverage
    {
    private:
        TYPE buffer[N] { };
        size_t index { 0 };
        TYPE sum { 0 };
    public:
        using type = TYPE;
    public:
        /**
         *  @brief  Default constructor for the MovingAverage class.
         */
        MovingAverage() = default;

        /**
         *  @brief  Add a new value to the moving average.
         *  @param  value The new value to add.
         */
        TYPE update(TYPE value)
        {
            sum -= buffer[index];
            buffer[index] = value;
            sum += value;
            index = (index + 1) % N;
            return sum / static_cast<TYPE>(N);
        }

        /**
         *  @brief  Fill the buffer with a specific value.
         */
        void fill_buffer(TYPE value)
        {
            for (size_t i = 0; i < N; ++i)
            {
                buffer[i] = value;
            }
            sum = value * static_cast<TYPE>(N);
        }
    };
}
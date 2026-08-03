#pragma once

#include "main.h"

#include "utils.hpp"

#include <type_traits>
#include <cstddef>
#include <expected>

namespace PUTM
{
    // using Utils::static_for;
    using Utils::throw_consteval_failure;

    /**
     *  @brief  Fixed-size bidirectional PCHIP interpolation class.
     *  @tparam `SIZE` Number of interpolation points.
     *  @tparam `TYPE` Arithmetic type used for interpolation.
     *  @tparam `INVERSE_ITERATIONS` Number of bisection iterations used for y -> x.
     *
     *  @note   The x coordinates must be strictly increasing.
     *  @note   The y coordinates must be monotonic for y -> x interpolation.
     *  @note   PCHIP tangents are internal implementation details.
     *  @note   No dynamic memory allocation is used.
     */
    template<size_t SIZE, class TYPE = float>
    requires (SIZE >= 2) and std::is_arithmetic_v<TYPE>
    class PCHIP
    {
    public:
        enum struct Monotonicity
        {
            None,
            MonotoneIncreasing,
            MonotoneDecreasing,
            StrictlyIncreasing,
            StrictlyDecreasing
        };
    private:
        mutable TYPE __x[SIZE] { };
        mutable TYPE __y[SIZE] { };

        /*
         * Internal PCHIP tangents used for x -> y interpolation.
         */
        mutable TYPE __tangents_x[SIZE] { };
        mutable TYPE __tangents_y[SIZE] { };

        mutable bool __initialized { false };
        /* more so sanity check */
        mutable bool __consteval { false };

        mutable Monotonicity __monotonicity { Monotonicity::None };
        // bool __is_strictly_monotone_increasing { false };
        // bool __is_strictly_monotone_decreasing { false };
    
    public:
        using type = TYPE;
    public:
        /**
         *  @brief  Default constructor.
         *  @note   set_data() must be called before interpolation.
         */
        constexpr PCHIP() = default;
    public:
        /**
         *  @brief  Construct an interpolator from fixed-size x and y arrays.
         *  @param  x Input x coordinates.
         *  @param  y Input y coordinates.
         */
        constexpr PCHIP(const TYPE (&x)[SIZE], const TYPE (&y)[SIZE])
        {
            if consteval
            {
                __consteval = true;
            }
            __initialized = set_data(x, y);
        }
    public:
        /**
         *  @brief  Set the interpolation data.
         *  @param  x Input x coordinates. Must be strictly increasing.
         *  @param  y Input y coordinates.
         *  @return true when x -> y interpolation can be initialized.
         *
         *  @note   y -> x is enabled when y is monotonically increasing or
         *          monotonically decreasing and is not constant.
         */
        constexpr bool set_data(const TYPE (&x)[SIZE], const TYPE (&y)[SIZE])
        {   
            bool is_monotone_increasing = true;
            bool is_monotone_decreasing = true;
            bool is_strictly_increasing = true;
            bool is_strictly_decreasing = true;

            for (size_t i = 0; i < SIZE; ++i)
            {
                if(i != SIZE - 1) 
                {
                    if(x[i] >= x[i + 1]) 
                    {
                        if consteval { Utils::throw_consteval_failure("x not strictly increasing"); }
                        return false;
                    }
                    if(y[i] >= y[i + 1]) is_strictly_increasing = false;
                    if(y[i] <= y[i + 1]) is_strictly_decreasing = false;
                    if(y[i] > y[i + 1]) is_monotone_increasing = false;
                    if(y[i] < y[i + 1]) is_monotone_decreasing = false;
                }
                __x[i] = x[i];
                __y[i] = y[i];
                __tangents_x[i] = { };
                __tangents_y[i] = { };
            }

            if(is_strictly_increasing) __monotonicity = Monotonicity::StrictlyIncreasing;
            else if(is_strictly_decreasing) __monotonicity = Monotonicity::StrictlyDecreasing;
            else if(is_monotone_increasing) __monotonicity = Monotonicity::MonotoneIncreasing;
            else if(is_monotone_decreasing) __monotonicity = Monotonicity::MonotoneDecreasing;

            calculate_tangents();

            return true;
        }

    public:
        /**
         *  @brief  Interpolate y from x.
         *  @param  x Requested x coordinate.
         *  @param  output Interpolated y value.
         *  @return true when the interpolator is initialized.
         *
         *  @note   Queries outside the x range are clamped.
         */
        constexpr bool evaluate_y(TYPE x, TYPE& output) const
        {
            if (not __initialized)
            {
                output = { };
                return false;
            }

            output = interpolate_y(x);
            return true;
        }
    public:
        /**
         *  @brief  Interpolate x from y.
         *  @param  y Requested y coordinate.
         *  @param  output Interpolated x value.
         *  @return true when the y data has a valid monotonic inverse.
         *
         *  @note   Queries outside the y range are clamped.
         *  @note   For a flat y section, the lowest matching x is returned.
         */
        constexpr bool evaluate_x(TYPE y, TYPE& output) const
        {
            if (not can_get_x())
            {
                output = { };
                return false;
            }

            output = interpolate_x(y);
            return true;
        }
    public:
        /**
         *  @brief  Check whether x -> y interpolation is available.
         */
        constexpr bool is_initialized() const
        {
            return __initialized;
        }
    public:
        /**
         *  @brief  Check whether y -> x interpolation is available.
         */
        constexpr bool can_get_x() const
        {
            return __initialized and 
                  (__monotonicity == Monotonicity::StrictlyIncreasing or 
                   __monotonicity == Monotonicity::StrictlyDecreasing);
        }
    public:
        /**
         *  @brief  Get the number of interpolation points.
         */
        static consteval size_t size()
        {
            return SIZE;
        }
    private:
        static constexpr int sign(TYPE value)
        {
            return (value > static_cast<TYPE>(0)) - (value < static_cast<TYPE>(0));
        }
    private:
        static constexpr TYPE absolute(TYPE value)
        {
            return value < static_cast<TYPE>(0) ? -value : value;
        }
    private:
        static constexpr TYPE endpoint_tangent(TYPE h0, TYPE h1, TYPE delta0, TYPE delta1)
        {
            TYPE result = ((static_cast<TYPE>(2) * h0 + h1) * delta0 - h0 * delta1) / (h0 + h1);

            if (sign(result) != sign(delta0))
            {
                return static_cast<TYPE>(0);
            }

            if ((sign(delta0) != sign(delta1)) and (absolute(result) > absolute(static_cast<TYPE>(3) * delta0)))
            {
                return static_cast<TYPE>(3) * delta0;
            }

            return result;
        }
    private:
        constexpr void calculate_tangents()
        {
            if constexpr (SIZE == 2)
            {
                {
                    const TYPE tangent_x = (__y[1] - __y[0]) / (__x[1] - __x[0]);

                    __tangents_x[0] = tangent_x;
                    __tangents_x[1] = tangent_x;
                }
                if(can_get_x())
                {
                    const TYPE tangent_y = (__x[1] - __x[0]) / (__y[1] - __y[0]);

                    __tangents_y[0] = tangent_y;
                    __tangents_y[1] = tangent_y;
                }
                return;
            }

            {
                const TYPE x_first_h0 = __x[1] - __x[0];
                const TYPE x_first_h1 = __x[2] - __x[1];
                const TYPE x_first_delta0 = (__y[1] - __y[0]) / x_first_h0;
                const TYPE x_first_delta1 = (__y[2] - __y[1]) / x_first_h1;

                __tangents_x[0] = endpoint_tangent(x_first_h0, x_first_h1, x_first_delta0, x_first_delta1);

                for (size_t i = 1; i < SIZE - 1; i++)
                {
                    const TYPE x_prev_h = __x[i] - __x[i - 1];
                    const TYPE x_next_h = __x[i + 1] - __x[i];
                    const TYPE x_prev_delta = (__y[i] - __y[i - 1]) / x_prev_h;
                    const TYPE x_next_delta = (__y[i + 1] - __y[i]) / x_next_h;

                    if ((x_prev_delta == static_cast<TYPE>(0))
                        or (x_next_delta == static_cast<TYPE>(0))
                        or (sign(x_prev_delta) != sign(x_next_delta)))
                    {
                        __tangents_x[i] = static_cast<TYPE>(0);
                    }
                    else
                    {
                        const TYPE x_weight1 = static_cast<TYPE>(2) * x_next_h + x_prev_h;
                        const TYPE x_weight2 = x_next_h + static_cast<TYPE>(2) * x_prev_h;

                        __tangents_x[i] = (x_weight1 + x_weight2) / ((x_weight1 / x_prev_delta) + 
                                          (x_weight2 / x_next_delta));
                    }
                }

                const size_t last = SIZE - 1;

                const TYPE x_last_h0 =  __x[last] - __x[last - 1];
                const TYPE x_last_h1 = __x[last - 1] - __x[last - 2];
                const TYPE x_last_delta0 = (__y[last] - __y[last - 1]) / x_last_h0;
                const TYPE x_last_delta1 = (__y[last - 1] - __y[last - 2]) / x_last_h1;

                __tangents_x[last] = endpoint_tangent(x_last_h0, x_last_h1, x_last_delta0, x_last_delta1);
            }

            if(can_get_x())
            {
                const TYPE y_first_h0 = __x[1] - __x[0];
                const TYPE y_first_h1 = __x[2] - __x[1];
                const TYPE y_first_delta0 = (__y[1] - __y[0]) / y_first_h0;
                const TYPE y_first_delta1 = (__y[2] - __y[1]) / y_first_h1;

                __tangents_y[0] = endpoint_tangent(y_first_h0, y_first_h1, y_first_delta0, y_first_delta1);

                for (size_t i = 1; i < SIZE - 1; i++)
                {
                    const TYPE y_prev_h = __x[i] - __x[i - 1];
                    const TYPE y_next_h = __x[i + 1] - __x[i];
                    const TYPE y_prev_delta = (__y[i] - __y[i - 1]) / y_prev_h;
                    const TYPE y_next_delta = (__y[i + 1] - __y[i]) / y_next_h;

                    if ((y_prev_delta == static_cast<TYPE>(0))
                        or (y_next_delta == static_cast<TYPE>(0))
                        or (sign(y_prev_delta) != sign(y_next_delta)))
                    {
                        __tangents_y[i] = static_cast<TYPE>(0);
                    }
                    else
                    {
                        const TYPE y_weight1 = static_cast<TYPE>(2) * y_next_h + y_prev_h;
                        const TYPE y_weight2 = y_next_h + static_cast<TYPE>(2) * y_prev_h;

                        __tangents_y[i] = (y_weight1 + y_weight2) / ((y_weight1 / y_prev_delta) + 
                                          (y_weight2 / y_next_delta));
                    }
                }

                const size_t last = SIZE - 1;

                const TYPE y_last_h0 =  __y[last] - __y[last - 1];

                const TYPE y_last_h1 = __y[last - 1] - __y[last - 2];

                const TYPE y_last_delta0 = (__x[last] - __x[last - 1]) / y_last_h0;

                const TYPE y_last_delta1 = (__x[last - 1] - __x[last - 2]) / y_last_h1;

                __tangents_y[last] = endpoint_tangent(y_last_h0, y_last_h1, y_last_delta0, y_last_delta1);
            }
        }
    private:
        constexpr size_t find_x_interval(TYPE query) const
        {
            size_t low = 0;
            size_t high = SIZE - 1;

            while ((high - low) > 1)
            {
                const size_t middle = low + (high - low) / 2;

                if (query < __x[middle])
                {
                    high = middle;
                }
                else
                {
                    low = middle;
                }
            }

            return low;
        }
    private:
        constexpr size_t find_y_interval(TYPE query) const
        {
            size_t low = 0;
            size_t high = SIZE - 1;

            while ((high - low) > 1)
            {
                const size_t middle = low + (high - low) / 2;

                if (__monotonicity == Monotonicity::StrictlyIncreasing)
                {
                    if (__y[middle] >= query)
                    {
                        high = middle;
                    }
                    else
                    {
                        low = middle;
                    }
                }
                /* assume __is_strictly_monotone_decreasing -> this function 
                 * shouldn't be reachable otherwise */
                else
                {
                    if (__y[middle] <= query)
                    {
                        high = middle;
                    }
                    else
                    {
                        low = middle;
                    }
                }
            }

            return low;
        }
    private:
        constexpr TYPE evaluate_interval_x(size_t interval, TYPE query) const
        {
            const TYPE x0 = __x[interval];
            const TYPE x1 = __x[interval + 1];
            const TYPE width = x1 - x0;

            const TYPE t = (query - x0) / width;
            const TYPE t2 = t * t;
            const TYPE t3 = t2 * t;

            const TYPE h00 = static_cast<TYPE>(2) * t3 - static_cast<TYPE>(3) * t2 +
                             static_cast<TYPE>(1);

            const TYPE h10 = t3 - static_cast<TYPE>(2) * t2 + t;

            const TYPE h01 = -static_cast<TYPE>(2) * t3 + static_cast<TYPE>(3) * t2;

            const TYPE h11 = t3 - t2;

            return h00 * __y[interval] +
                   h10 * width * __tangents_x[interval] +
                   h01 * __y[interval + 1] +
                   h11 * width * __tangents_x[interval + 1];
        }
    private:
        constexpr TYPE evaluate_interval_y(size_t interval, TYPE query) const
        {
            const TYPE y0 = __y[interval];
            const TYPE y1 = __y[interval + 1];
            const TYPE width = y1 - y0;

            const TYPE t = (query - y0) / width;
            const TYPE t2 = t * t;
            const TYPE t3 = t2 * t;

            const TYPE h00 = static_cast<TYPE>(2) * t3 - static_cast<TYPE>(3) * t2 +
                             static_cast<TYPE>(1);

            const TYPE h10 = t3 - static_cast<TYPE>(2) * t2 + t;

            const TYPE h01 = -static_cast<TYPE>(2) * t3 + static_cast<TYPE>(3) * t2;

            const TYPE h11 = t3 - t2;

            return h00 * __x[interval] +
                   h10 * width * __tangents_y[interval] +
                   h01 * __x[interval + 1] +
                   h11 * width * __tangents_y[interval + 1];
        }
    private:
        constexpr TYPE interpolate_y(TYPE query) const
        {
            if (query <= __x[0])
            {
                return __y[0];
            }

            if (query >= __x[SIZE - 1])
            {
                return __y[SIZE - 1];
            }

            return evaluate_interval_x(find_x_interval(query), query);
        }
    private:
        constexpr TYPE interpolate_x(TYPE query) const
        {
            if (__monotonicity == Monotonicity::StrictlyIncreasing)
            {
                if (query <= __y[0])
                {
                    return __x[0];
                }

                if (query >= __y[SIZE - 1])
                {
                    return __x[SIZE - 1];
                }
            }
            /* assume __is_strictly_monotone_decreasing -> this function 
             * shouldn't be reachable otherwise */
            else 
            {
                if (query >= __y[0])
                {
                    return __x[0];
                }

                if (query <= __y[SIZE - 1])
                {
                    return __x[SIZE - 1];
                }
            }

            return evaluate_interval_y(find_y_interval(query), query);
        }
    };
}

//FIXME: delete this later
// static constexpr PUTM::PCHIP temporary { {1, 2, 3, 4}, {1, 2, 3, 4} };
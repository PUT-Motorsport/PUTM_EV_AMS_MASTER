// #pragma once

// #include "main.h"

// #include "cstddef"
// #include "type_traits"
// #include "utils.hpp"

// namespace PUTM
// {
//     using Utils::static_for;
//     using Utils::throw_consteval_failure;

//     /**
//      *  @brief  Fixed-size bidirectional PCHIP interpolation class.
//      *  @tparam N Number of interpolation points.
//      *  @tparam TYPE Arithmetic type used for interpolation.
//      *  @tparam INVERSE_ITERATIONS Number of bisection iterations used for y -> x.
//      *
//      *  @note   The x coordinates must be strictly increasing.
//      *  @note   The y coordinates must be monotonic for y -> x interpolation.
//      *  @note   PCHIP tangents are internal implementation details.
//      *  @note   No dynamic memory allocation is used.
//      */
//     template<size_t SIZE, class TYPE = float, size_t INVERSE_ITERATIONS = 20>
//     requires (SIZE >= 2) and (INVERSE_ITERATIONS > 0) and std::is_arithmetic_v<TYPE>
//     class PCHIP
//     {
//     private:
//         TYPE __x[SIZE] { };
//         TYPE __y[SIZE] { };

//         /*
//          * Internal PCHIP tangents used for x -> y interpolation.
//          * They are intentionally not exposed through the public interface.
//          */
//         TYPE __tangents[SIZE] { };

//         bool __initialized { false };
//         bool __is_strictly_monotone { false };
    
//     public:
//         using type = TYPE;
//     public:
//         /**
//          *  @brief  Default constructor.
//          *  @note   set_data() must be called before interpolation.
//          */
//         constexpr PCHIP() = default;

//         /**
//          *  @brief  Construct an interpolator from fixed-size x and y arrays.
//          *  @param  x Input x coordinates.
//          *  @param  y Input y coordinates.
//          */
//         constexpr PCHIP(const TYPE (&x)[SIZE], const TYPE (&y)[SIZE])
//         {
//             if consteval
//             {
//                 __is_strictly_monotone = true;

//                 static_for<0, SIZE>([&]<int I>() 
//                 {
//                     if(I != SIZE - 1) if(x[I] >= x[I + 1]) throw_consteval_failure("x not strictly increasing");
//                     TYPE 
//                     if(I != SIZE - 1) if(y[I] == y[I + 1]) __is_strictly_monotone
//                     __x[I] = x[I];
//                     __y[I] = y[I];
//                     // __tangents[I] = { };
//                 });
                
//                 if (SIZE == 2)
//                 {
//                     const TYPE tangent = (__y[1] - __y[0]) / (__x[1] - __x[0]);

//                     __tangents[0] = tangent;
//                     __tangents[1] = tangent;
//                 }
//                 else
//                 {
//                     const TYPE h0 = __x[1] - __x[0];
//                     const TYPE h1 = __x[2] - __x[1];

//                     const TYPE delta0 = (__y[1] - __y[0]) / h0;

//                     const TYPE delta1 = (__y[2] - __y[1]) / h1;

//                     __tangents[0] = endpoint_tangent(h0, h1, delta0, delta1);

//                     static_for<0, SIZE>([&]<int I>() 
//                     {
//                         const TYPE prev_h = __x[I] - __x[I - 1];

//                         const TYPE next_h = __x[I + 1] - __x[I];

//                         const TYPE prev_delta = (__y[I] - __y[I - 1]) / prev_h;

//                         const TYPE next_delta = (__y[I + 1] - __y[I]) / next_h;

//                         if ((prev_delta == static_cast<TYPE>(0)) or (next_delta == static_cast<TYPE>(0))
//                              or (sign(prev_delta) != sign(next_delta)))
//                         {
//                             __tangents[I] = static_cast<TYPE>(0);
//                         }
//                         else
//                         {
//                             const TYPE weight1 = static_cast<TYPE>(2) * next_h + previous_h;

//                             const TYPE weight2 = next_h + static_cast<TYPE>(2) * previous_h;

//                             __tangents[I] = (weight1 + weight2) / ((weight1 / prev_delta) 
//                                             + (weight2 / next_delta));
//                         }
//                     })

//                     const size_t last = SIZE - 1;

//                     const TYPE last_h0 = __x[last] - __x[last - 1];

//                     const TYPE last_h1 = __x[last - 1] - __x[last - 2];

//                     const TYPE last_delta0 = (__y[last] - __y[last - 1]) / last_h0;

//                     const TYPE last_delta1 = (__y[last - 1] - __y[last - 2]) / last_h1;

//                     __tangents[last] = endpoint_tangent(last_h0, last_h1, last_delta0, last_delta1);
//                 }
//             }
//         }
//     public:
//         /**
//          *  @brief  Set the interpolation data.
//          *  @param  x Input x coordinates. Must be strictly increasing.
//          *  @param  y Input y coordinates.
//          *  @return true when x -> y interpolation can be initialized.
//          *
//          *  @note   y -> x is enabled when y is monotonically increasing or
//          *          monotonically decreasing and is not constant.
//          */
//         constexpr bool set_data(const TYPE (&x)[SIZE], const TYPE (&y)[SIZE])
//         {   
//                 for (size_t i = 0; i < N; ++i)
//                 {
//                     x_values[i] = x[i];
//                     y_values[i] = y[i];
//                     tangents[i] = { };
//                 }

//                 if (!is_strictly_increasing(x_values))
//                 {
//                     return false;
//                 }

//                 calculate_tangents();
//                 initialized = true;

//                 bool contains_increase = false;
//                 bool contains_decrease = false;

//                 for (size_t i = 0; i < N - 1; ++i)
//                 {
//                     if (y_values[i + 1] > y_values[i])
//                     {
//                         contains_increase = true;
//                     }
//                     else if (y_values[i + 1] < y_values[i])
//                     {
//                         contains_decrease = true;
//                     }
//                 }

//                 /*
//                 * Flat sections are permitted, but a completely constant y array
//                 * cannot have a unique inverse.
//                 */
//                 inverse_available =
//                     !(contains_increase && contains_decrease)
//                     && (contains_increase || contains_decrease);

//                 y_increasing = contains_increase;

//                 return true;
//         }

//     public:
//         /**
//          *  @brief  Interpolate y from x.
//          *  @param  x Requested x coordinate.
//          *  @param  output Interpolated y value.
//          *  @return true when the interpolator is initialized.
//          *
//          *  @note   Queries outside the x range are clamped.
//          */
//         constexpr bool get_y(
//             TYPE x,
//             TYPE& output) const
//         {
//             if (!initialized)
//             {
//                 output = { };
//                 return false;
//             }

//             output = interpolate_y(x);
//             return true;
//         }

//         /**
//          *  @brief  Interpolate y from x.
//          *  @param  x Requested x coordinate.
//          *  @return Interpolated y value, or zero when uninitialized.
//          */
//         constexpr TYPE get_y(TYPE x) const
//         {
//             if (!initialized)
//             {
//                 return { };
//             }

//             return interpolate_y(x);
//         }

//         /**
//          *  @brief  Interpolate x from y.
//          *  @param  y Requested y coordinate.
//          *  @param  output Interpolated x value.
//          *  @return true when the y data has a valid monotonic inverse.
//          *
//          *  @note   Queries outside the y range are clamped.
//          *  @note   For a flat y section, the lowest matching x is returned.
//          */
//         constexpr bool get_x(TYPE y, TYPE& output) const
//         {
//             if (!initialized || !inverse_available)
//             {
//                 output = { };
//                 return false;
//             }

//             output = interpolate_x(y);
//             return true;
//         }

//         /**
//          *  @brief  Interpolate x from y.
//          *  @param  y Requested y coordinate.
//          *  @return Interpolated x value, or zero when no inverse is available.
//          */
//         constexpr TYPE get_x(TYPE y) const
//         {
//             return interpolate_x(y);
//         }

//         /**
//          *  @brief  Alias for x -> y interpolation.
//          */
//         constexpr TYPE evaluate(TYPE x) const
//         {
//             return get_y(x);
//         }

//         /**
//          *  @brief  Function-call syntax for x -> y interpolation.
//          */
//         constexpr TYPE operator()(TYPE x) const
//         {
//             return get_y(x);
//         }

//     public:
//         /**
//          *  @brief  Check whether x -> y interpolation is available.
//          */
//         constexpr bool is_initialized() const
//         {
//             return __initialized;
//         }

//         /**
//          *  @brief  Check whether y -> x interpolation is available.
//          */
//         constexpr bool can_get_x() const
//         {
//             return __initialized && __is_strictly_monotone;
//         }

//         /**
//          *  @brief  Get the number of interpolation points.
//          */
//         static consteval size_t size()
//         {
//             return SIZE;
//         }

//         /**
//          *  @brief  Access a stored x coordinate.
//          *  @param  index Data-point index.
//          */
//         // constexpr const TYPE& x(size_t index) const
//         // {
//         //     return __x[index];
//         // }

//         /**
//          *  @brief  Access a stored y coordinate.
//          *  @param  index Data-point index.
//          */
//         // constexpr const TYPE& y(size_t index) const
//         // {
//         //     return __y[index];
//         // }

//     private:
//         static constexpr int sign(TYPE value)
//         {
//             return (value > static_cast<TYPE>(0)) - (value < static_cast<TYPE>(0));
//         }

//         static constexpr TYPE absolute(TYPE value)
//         {
//             return value < static_cast<TYPE>(0) ? -value : value;
//         }

//         static constexpr TYPE endpoint_tangent(TYPE h0, TYPE h1, TYPE delta0, TYPE delta1)
//         {
//             TYPE result = ((static_cast<TYPE>(2) * h0 + h1) * delta0 - h0 * delta1) / (h0 + h1);

//             if (sign(result) != sign(delta0))
//             {
//                 return static_cast<TYPE>(0);
//             }

//             if ((sign(delta0) != sign(delta1)) and (absolute(result) > absolute(static_cast<TYPE>(3) * delta0)))
//             {
//                 return static_cast<TYPE>(3) * delta0;
//             }

//             return result;
//         }

//         constexpr void calculate_tangents()
//         {
//             // if constexpr (N == 2)
//             // {
//             //     const TYPE tangent =
//             //         (y_values[1] - y_values[0])
//             //         / (x_values[1] - x_values[0]);

//             //     tangents[0] = tangent;
//             //     tangents[1] = tangent;
//             //     return;
//             // }

//             // const TYPE first_h0 = x_values[1] - x_values[0];
//             // const TYPE first_h1 = x_values[2] - x_values[1];

//             // const TYPE first_delta0 =
//             //     (y_values[1] - y_values[0]) / first_h0;

//             // const TYPE first_delta1 =
//             //     (y_values[2] - y_values[1]) / first_h1;

//             // tangents[0] = endpoint_tangent(
//             //     first_h0,
//             //     first_h1,
//             //     first_delta0,
//             //     first_delta1);

//             // for (size_t i = 1; i < N - 1; ++i)
//             // {
//             //     const TYPE previous_h =
//             //         x_values[i] - x_values[i - 1];

//             //     const TYPE next_h =
//             //         x_values[i + 1] - x_values[i];

//             //     const TYPE previous_delta =
//             //         (y_values[i] - y_values[i - 1])
//             //         / previous_h;

//             //     const TYPE next_delta =
//             //         (y_values[i + 1] - y_values[i])
//             //         / next_h;

//             //     if ((previous_delta == static_cast<TYPE>(0))
//             //         || (next_delta == static_cast<TYPE>(0))
//             //         || (sign(previous_delta) != sign(next_delta)))
//             //     {
//             //         tangents[i] = static_cast<TYPE>(0);
//             //     }
//             //     else
//             //     {
//             //         const TYPE weight1 =
//             //             static_cast<TYPE>(2) * next_h + previous_h;

//             //         const TYPE weight2 =
//             //             next_h + static_cast<TYPE>(2) * previous_h;

//             //         tangents[i] =
//             //             (weight1 + weight2)
//             //             / ((weight1 / previous_delta)
//             //                + (weight2 / next_delta));
//             //     }
//             // }

//             // const size_t last = N - 1;

//             // const TYPE last_h0 =
//             //     x_values[last] - x_values[last - 1];

//             // const TYPE last_h1 =
//             //     x_values[last - 1] - x_values[last - 2];

//             // const TYPE last_delta0 =
//             //     (y_values[last] - y_values[last - 1])
//             //     / last_h0;

//             // const TYPE last_delta1 =
//             //     (y_values[last - 1] - y_values[last - 2])
//             //     / last_h1;

//             // tangents[last] = endpoint_tangent(
//             //     last_h0,
//             //     last_h1,
//             //     last_delta0,
//             //     last_delta1);
//         }

//         constexpr size_t find_x_interval(TYPE query) const
//         {
//             // size_t low = 0;
//             // size_t high = N - 1;

//             // while ((high - low) > 1)
//             // {
//             //     const size_t middle = low + (high - low) / 2;

//             //     if (query < x_values[middle])
//             //     {
//             //         high = middle;
//             //     }
//             //     else
//             //     {
//             //         low = middle;
//             //     }
//             // }

//             return low;
//         }

//         constexpr size_t find_y_interval(TYPE query) const
//         {
//             // size_t low = 0;
//             // size_t high = N - 1;

//             // while ((high - low) > 1)
//             // {
//             //     const size_t middle = low + (high - low) / 2;

//             //     if (y_increasing)
//             //     {
//             //         if (y_values[middle] >= query)
//             //         {
//             //             high = middle;
//             //         }
//             //         else
//             //         {
//             //             low = middle;
//             //         }
//             //     }
//             //     else
//             //     {
//             //         if (y_values[middle] <= query)
//             //         {
//             //             high = middle;
//             //         }
//             //         else
//             //         {
//             //             low = middle;
//             //         }
//             //     }
//             // }

//             // return low;
//         }

//         constexpr TYPE evaluate_interval(
//             size_t interval,
//             TYPE query) const
//         {
//             // const TYPE x0 = x_values[interval];
//             // const TYPE x1 = x_values[interval + 1];
//             // const TYPE width = x1 - x0;

//             // const TYPE t = (query - x0) / width;
//             // const TYPE t2 = t * t;
//             // const TYPE t3 = t2 * t;

//             // const TYPE h00 =
//             //     static_cast<TYPE>(2) * t3
//             //     - static_cast<TYPE>(3) * t2
//             //     + static_cast<TYPE>(1);

//             // const TYPE h10 =
//             //     t3 - static_cast<TYPE>(2) * t2 + t;

//             // const TYPE h01 =
//             //     -static_cast<TYPE>(2) * t3
//             //     + static_cast<TYPE>(3) * t2;

//             // const TYPE h11 = t3 - t2;

//             // return
//             //     h00 * y_values[interval]
//             //     + h10 * width * tangents[interval]
//             //     + h01 * y_values[interval + 1]
//             //     + h11 * width * tangents[interval + 1];
//         }

//         constexpr TYPE interpolate_y(TYPE query) const
//         {
//             // if (query <= x_values[0])
//             // {
//             //     return y_values[0];
//             // }

//             // if (query >= x_values[N - 1])
//             // {
//             //     return y_values[N - 1];
//             // }

//             // return evaluate_interval(
//             //     find_x_interval(query),
//             //     query);
//         }

//         constexpr TYPE interpolate_x(TYPE query) const
//         {
//             // if (y_increasing)
//             // {
//             //     if (query <= y_values[0])
//             //     {
//             //         return x_values[0];
//             //     }

//             //     if (query >= y_values[N - 1])
//             //     {
//             //         return x_values[N - 1];
//             //     }
//             // }
//             // else
//             // {
//             //     if (query >= y_values[0])
//             //     {
//             //         return x_values[0];
//             //     }

//             //     if (query <= y_values[N - 1])
//             //     {
//             //         return x_values[N - 1];
//             //     }
//             // }

//             // const size_t interval = find_y_interval(query);

//             /*
//              * Invert the already-defined PCHIP segment by bisection.
//              * No second tangent/derivative table is required.
//              */
//             // TYPE left = x_values[interval];
//             // TYPE right = x_values[interval + 1];

//             for (size_t i = 0; i < INVERSE_ITERATIONS; ++i)
//             {
//                 const TYPE middle =
//                     (left + right) / static_cast<TYPE>(2);

//                 const TYPE middle_y =
//                     evaluate_interval(interval, middle);

//                 if (true)
//                 {
//                     if (middle_y < query)
//                     {
//                         left = middle;
//                     }
//                     else
//                     {
//                         right = middle;
//                     }
//                 }
//                 else
//                 {
//                     if (middle_y > query)
//                     {
//                         left = middle;
//                     }
//                     else
//                     {
//                         right = middle;
//                     }
//                 }
//             }

//             return (left + right) / static_cast<TYPE>(2);
//         }
//     };
// }

#pragma once

#include "main.h"

#include "array"
#include "type_traits"

#include "utils.hpp"

namespace PUTM
{
    /**
     *  @brief  A simple polynomial class for polynomial operations.
     *  @tparam N The degree of the polynomial.
     *  @tparam TYPE The type of the coefficients.
     *  @note   This class is designed to be used with fixed-size polynomials.
     *          It does not support dynamic resizing or complex operations.
     *          The coeficients go from N to 0 in order
     */
    template<size_t N, class TYPE = float>
    requires (N > 0) && std::is_arithmetic_v<TYPE>
    class Polynomial
    {
    private:
        TYPE coeffs[N] { };
    public:
        using type = TYPE;
    public:
        /**
         *  @brief  Default constructor for the Polynomial class.
         *  @note   Initializes all coefficients to zero.
         */
        Polynomial() = default;
    public:
        /**
         *  @brief  Copy constructor for the Polynomial class.
         *  @param  other The polynomial to copy from.
         */
        consteval Polynomial(const TYPE (&coeffs)[N])
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
         *  @brief  Constructor for the Polynomial class.
         *  @param  coeffs An array of coefficients to initialize the polynomial.
         *  @note   This constructor allows initializing the polynomial with a
         *          predefined array of coefficients. The oreder of the coefficients
         *          should be from the highest degree to the lowest degree.
         */
        // template<typename ... ARGS>
        // consteval Polynomial(const ARGS&&... coeffs);
    public:
        /**
         *  @brief  Access a coefficient in the polynomial.
         *  @param  index The index of the coefficient.
         *  @return A reference to the coefficient at the specified index.
         *  @note   This function is not constexpr because it returns a reference
         *          to the coefficient.
         */
        constexpr TYPE &operator[](size_t index)
        {
            return coeffs[index];
        }
    public:
        /**
         *  @brief  Evaluate the polynomial at a given point.
         *  @param  x The point at which to evaluate the polynomial.
         *  @return The value of the polynomial at the specified point.
         *  @note   This function is constexpr, allowing it to be evaluated at compile time.
         *          It uses Horner's method for efficient evaluation.
         */
        constexpr TYPE evaluate(TYPE x) const
        {
            TYPE result = coeffs[0];
            for (size_t i = 1; i < N; ++i)
            {
                result = result * x + coeffs[i];
            }
            return result;
        }
    public:
        /**
         *  @brief  Get the degree of the polynomial.
         *  @return The degree of the polynomial.
         *  @note   This function is consteval, allowing it to be evaluated at compile time.
         */
        consteval size_t degree() const
        {
            return N - 1;
        }
    public:
        /**
         *  @brief  Get the derivative of the polynomial.
         *  @return A new polynomial representing the derivative of the original polynomial.
         *  @note   This function is consteval, allowing it to be evaluated at compile time.
         */
        consteval Polynomial<N-1, TYPE> derivative() const
        {
            Polynomial<N - 1, TYPE> result;
            static_for<1, N>([&]<int I>() 
            {
                result[I - 1] = coeffs[I] * I;
            });
            return result;
        }
    };

    // template<>
    // template<typename ... ARGS>
    // consteval Polynomial<sizeof...(ARGS), std::common_type_t<ARGS...>>::Polynomial(const ARGS&&... coeffs)
    // {
    //     // Constructor body is empty, initialization is done in the member initializer list
    // }

    template<size_t N, class TYPE>
    constexpr TYPE newton_raphson(const Polynomial<N, TYPE> &polynomial, const Polynomial<N - 1, TYPE> &derivative, TYPE x0, TYPE y = { }, TYPE tolerance = 1e-3, size_t max_iter = 40)
    {
        //Polynomial derivative = poly.derivative();
        TYPE x = x0;
        TYPE fx = polynomial.evaluate(x);
        TYPE dfx = derivative.evaluate(x);
        while (std::abs(fx) > tolerance and max_iter-- > 0)
        {
            x -= fx / dfx;
            fx = polynomial.evaluate(x) - y;
            dfx = derivative.evaluate(x);
        }
        return x;
    }
}
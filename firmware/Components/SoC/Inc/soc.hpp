#pragma once

#include "main.h"
#include "cmath"
#include "algorithm"

#include "matrix.hpp"
#include "polynomial.hpp"
#include "config.hpp"

namespace PUTM
{
    class SoC
    {
    private:
        // static constexpr float Ts = 0.1f; // Sampling time in seconds
    public:
        SoC() = default;
        ~SoC() = default;

        SoC(const SoC &) = delete;
        SoC &operator=(const SoC &) = delete;

        SoC(SoC &&) = delete;
        SoC &operator=(SoC &&) = delete;
    public:
        // using Matrix3x3 = Matrix<3, 3, float>;
        // using VectorCol = Matrix<3, 1, float>;
        // using VectorRow = Matrix<1, 3, float>;
    private:
        // TODO?: last row looks sus
        // TODO: parametrize this shit
        static inline constexpr Matrix A {{{ 1.00000000f, 0.00000000f,  0.00000000f }, 
                                           { 0.00000000f, Config::A1, Config::A2 }, 
                                           { 0.00000000f, 1.00000000f,  0.00000000f }}};
        static inline constexpr Matrix B {{{ -Config::DT / Config::CELL_NOMINAL_CAPACITY / 60.0f / 60.0f },
                                           { 1.00000000f },
                                           { 0.00000000f }}};
        static inline constexpr Matrix C {{{ 0.00000000f, Config::C1, Config::C2 }}};
        // static inline constexpr float D { 0.00698732f };
        static inline constexpr float D { Config::D1 }; 
        static inline constexpr Matrix Q {{{ 0.00020000f, 0.00000000f, 0.00000000f },
                                           { 0.00000000f, 0.02000000f, 0.00000000f },
                                           { 0.00000000f, 0.00000000f, 0.02000000f }}};
        static inline constexpr float R { 2500000.0f };
        // was + but * ?
        static inline constexpr float R_charging { R * 99.f };
        static inline constexpr Polynomial ocv { Config::POLYNOMIAL_OCV };
        static inline constexpr Polynomial docv = ocv.derivative();

        Matrix<3,3> P {{{ 20.0000000f, 0.00000000f, 0.00000000f }, 
                        { 0.00000000f, 2.00000000f, 0.00000000f }, 
                        { 0.00000000f, 0.00000000f, 2.00000000f }}};
        Matrix<3, 1> x {{{ 0.98f }, 
                         { 0.00f }, 
                         { 0.00f }}};

    private:
        void predict(float current)
        {
            x = A * x + B * current;
            P = A * P * A.T() + Q;
        }
        void correct(float voltage, float current, bool charging)
        {
            float v_ocv = ocv.evaluate(x.at(0));
            float v_drop = dot(C, x) + D * current;
            float v_bat_model = v_ocv - v_drop;
            float d_ocv_d_soc = docv.evaluate(x.at(0));

            Matrix H {{{ d_ocv_d_soc, C.at(1), C.at(2) }}};

            float innovation = voltage - v_bat_model;

            float S = dot(H * P, H.T()) + R + R_charging * charging;

            float inv_S = 1.0f / S;

            Matrix K = P * H.T() * inv_S;

            x = x + K * innovation;
            P = (eye<3>() - K * H) * P;
        }
    public:
        /**
         *  @brief  Update the state of charge using the Kalman filter.
         *  @param  voltage The voltage value to use for the update.
         *  @param  current The current value to use for the update.
         *  @param  charging A boolean indicating whether the battery is charging or not.
         *  @note   This function updates the state of charge using the Kalman filter
         *          algorithm. It first predicts the next state and then corrects it
         *          using the given voltage and current values.
         */
        void update(float voltage, float current, bool charging)
        {
            predict(current);
            correct(voltage, current, charging);
            x.at(0) = std::clamp(x.at(0), 0.0f, 1.0f);  
        }
        /**
         *  @brief  Set the state of charge.
         *  @param  soc The state of charge to set.
         *  @note   This function sets the first element of the state vector x to the given
         *          state of charge, clamped between 0.0 and 1.0.
         */
        void set(const float soc)
        {
            x.at(0) = std::clamp(soc, 0.0f, 1.0f);
        }
        /**
         *  @brief  Set the state of charge from a voltage value.
         *  @param  voltage The voltage value to set the state of charge from.
         *  @note   This function uses the Newton-Raphson method to find the state of charge
         *          corresponding to the given voltage. The result is clamped between 0.0 and 1.0.
         */
        void set_from_voltage(float voltage)
        {
            auto soc = newton_raphson(ocv, docv, 0.5f, voltage);
            x.at(0) = std::clamp(soc, 0.0f, 1.0f);
        }
        /**
         *  @brief  Get the state of charge.
         *  @return The state of charge as a float value.
         *  @note   This function returns the first element of the state vector x.
         */
        float get() const
        {
            return x.at(0);
        }
    };
}
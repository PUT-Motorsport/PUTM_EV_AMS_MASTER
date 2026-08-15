#pragma once

#include "main.h"
#include "matrix.hpp"
#include "config.hpp"
#include "pchip.hpp"

#include <cmath>
#include <algorithm>

namespace PUTM
{
class SoC
{
private:
    using State = Matrix<5, 1>;
    using Mat  = Matrix<5, 5>;
    using HMat  = Matrix<1, 5>;

    float __innovation;
    float __v_model;
    float __k_soc;
    float __r0_i;

    // x = [SOC, V1, V2, Vh]^T
    State x {{
        { 0.5f },
        { 0.0f },
        { 0.0f },
        { 0.0f },
        { 0.0f }
    }};

    Mat P {{
        { 0.01f, 0.f, 0.f, 0.f, 0.f },
        { 0.f, 0.001f, 0.f, 0.f, 0.f },
        { 0.f, 0.f, 0.001f, 0.f, 0.f },
        { 0.f, 0.f, 0.f, 0.001f, 0.f },
        { 0.f, 0.f, 0.f, 0.f, 0.001f },

    }};

    static inline constexpr Mat Q {{
        { CONFIG::SOC::Q::SOC, 0.f, 0.f, 0.f, 0.f},
        { 0.f, CONFIG::SOC::Q::V1, 0.f, 0.f, 0.f },
        { 0.f, 0.f, CONFIG::SOC::Q::V2, 0.f, 0.f },
        { 0.f, 0.f, 0.f, CONFIG::SOC::Q::H, 0.f },
        { 0.f, 0.f, 0.f, 0.f, CONFIG::SOC::Q::R }
    }};

    static inline constexpr float R =
        CONFIG::SOC::R_MEASUREMENT;

    static inline constexpr PCHIP ocv {
        CONFIG::SOC::SOC,
        CONFIG::SOC::OCV_AVG,
        { .scale_y = 144.f }
    };

    static inline constexpr PCHIP ocv_hist {
        CONFIG::SOC::SOC,
        CONFIG::SOC::OCV_HIST,
        { .scale_y = 144.f }
    };

    template<PCHIPTemp T>
    static inline constexpr PCHIP R0 {
        CONFIG::SOC::SOC,
        CONFIG::SOC::R0<T>
    };

    template<PCHIPTemp T>
    static inline constexpr PCHIP R1 {
        CONFIG::SOC::SOC,
        CONFIG::SOC::R1<T>
    };

    template<PCHIPTemp T>
    static inline constexpr PCHIP C1 {
        CONFIG::SOC::SOC,
        CONFIG::SOC::C1<T>
    };

    template<PCHIPTemp T>
    static inline constexpr PCHIP R2 {
        CONFIG::SOC::SOC,
        CONFIG::SOC::R2<T>
    };

    template<PCHIPTemp T>
    static inline constexpr PCHIP C2 {
        CONFIG::SOC::SOC,
        CONFIG::SOC::C2<T>
    };

    static float sign(float x)
    {
        return static_cast<float>((x > 0.f) - (x < 0.f));
    }

    static float interp_temp(float v25, float v40, float temp)
    {
        temp = std::clamp(temp, 25.0f, 40.0f);

        return v25
            + (temp - 25.0f) / 15.0f
            * (v40 - v25);
    }

    static float derivative(const PCHIP<201>& table, float soc)
    {
        constexpr float ds = 0.001f;

        const float s1 = std::clamp(soc - ds, 0.0f, 1.0f);
        const float s2 = std::clamp(soc + ds, 0.0f, 1.0f);

        if (s1 == s2)
            return 0.0f;

        return (table.evaluate_y(s2) - table.evaluate_y(s1))
             / (s2 - s1);
    }


    void predict(float current, float temperature)
    {
        using enum PCHIPTemp;

        const float soc = x.at(0);
        const float v1  = x.at(1);
        const float v2  = x.at(2);

        // Parameters from lookup tables
        const float r1 = interp_temp(
            R1<_25degC>.evaluate_y(soc),
            R1<_40degC>.evaluate_y(soc),
            temperature
        );

        const float c1 = interp_temp(
            C1<_25degC>.evaluate_y(soc),
            C1<_40degC>.evaluate_y(soc),
            temperature
        );

        const float r2 = interp_temp(
            R2<_25degC>.evaluate_y(soc),
            R2<_40degC>.evaluate_y(soc),
            temperature
        );

        const float c2 = interp_temp(
            C2<_25degC>.evaluate_y(soc),
            C2<_40degC>.evaluate_y(soc),
            temperature
        );

        const float dt = CONFIG::SOC::DT;
        const float capacity = CONFIG::SOC::CAPACITY;

        const float a1 = std::exp(-dt / (r1 * c1));
        const float a2 = std::exp(-dt / (r2 * c2));

        const float alpha =
            dt / (3600.0f * capacity);

        const float ah = std::exp(
            -CONFIG::SOC::GAMMA
            * std::abs(current)
            * dt
            / (3600.0f * capacity)
        );

        const float hist =
            -sign(current) * ocv_hist.evaluate_y(soc);

        Mat A {{
            { 1.f, 0.f, 0.f, 0.f, 0.f },
            { 0.f, a1,  0.f, 0.f, 0.f },
            { 0.f, 0.f,  a2, 0.f, 0.f },
            { 0.f, 0.f, 0.f, ah, 0.f },
            { 0.f, 0.f, 0.f, 0.f, 1.f },
        }};

        State B {{
            { -alpha },
            { r1 * (1.f - a1) },
            { r2 * (1.f - a2) },
            { 0.f },
            { 0.f }
        }};

        State G {{
            { 0.f },
            { 0.f },
            { 0.f },
            { 1.f - ah }
        }};

        // State prediction
        x = A * x + B * current + G * hist;

        // EKF Jacobian
        Mat F = A;

        const float dr1 = interp_temp(
            derivative(R1<_25degC>, soc),
            derivative(R1<_40degC>, soc),
            temperature
        );

        const float dc1 = interp_temp(
            derivative(C1<_25degC>, soc),
            derivative(C1<_40degC>, soc),
            temperature
        );

        const float dr2 = interp_temp(
            derivative(R2<_25degC>, soc),
            derivative(R2<_40degC>, soc),
            temperature
        );

        const float dc2 = interp_temp(
            derivative(C2<_25degC>, soc),
            derivative(C2<_40degC>, soc),
            temperature
        );

        const float tau1 = r1 * c1;
        const float tau2 = r2 * c2;

        const float da1 =
            a1 * dt
            * (dr1 * c1 + r1 * dc1)
            / (tau1 * tau1);

        const float da2 =
            a2 * dt
            * (dr2 * c2 + r2 * dc2)
            / (tau2 * tau2);

        const float db1 =
            dr1 * (1.f - a1)
            - r1 * da1;

        const float db2 =
            dr2 * (1.f - a2)
            - r2 * da2;

        F.at(1, 0) = da1 * v1 + db1 * current;
        F.at(2, 0) = da2 * v2 + db2 * current;

        F.at(3, 0) =
            (1.f - ah)
            * sign(current)
            * derivative(ocv_hist, soc);

        // Covariance prediction
        P = F * P * F.T() + Q;
    }


    void correct(float voltage, float current, float temperature)
    {
        using enum PCHIPTemp;

        const float soc = x.at(0);
        const float v1  = x.at(1);
        const float v2  = x.at(2);
        const float vh  = x.at(3);
        const float dr  = x.at(4);

        const float r0 = interp_temp(
            R0<_25degC>.evaluate_y(soc),
            R0<_40degC>.evaluate_y(soc),
            temperature
        );

        __r0_i = r0 * current;

        // Predicted terminal voltage
        const float v_model =
            ocv.evaluate_y(soc)
            - v1
            - v2
            - (r0 + dr) * current
            + vh;
        
        __v_model = v_model;

        const float dr0 = interp_temp(
            derivative(R0<_25degC>, soc),
            derivative(R0<_40degC>, soc),
            temperature
        );

        HMat H {{
            {
                derivative(ocv, soc) - current * dr0,
                -1.f,
                -1.f,
                1.f,
                -current
            }
        }};

        const float innovation =
            voltage - v_model;

        __innovation = innovation;
        
        const float S =
            dot(H * P, H.T()) + R;

        State K =
            P * H.T() * (1.0f / S);

        __k_soc = K.at(0);

        // State correction
        x = x + K * innovation;

        // Covariance correction
        const Mat I = eye<5>();
        const Mat IKH = I - K * H;

        P =
            IKH * P * IKH.T()
            + (K * R) * K.T();

        
    }

    void init_from_voltage(float voltage)
    {
        const float soc =
            std::clamp(
                ocv.evaluate_x(voltage),
                0.0f,
                1.0f
            );

        x = {{
            {soc},
            {0.0f},
            {0.0f},
            {0.0f}
        }};

        P = {{
            {0.05f * 0.05f, 0.f, 0.f, 0.f},
            {0.f, 0.01f * 0.01f, 0.f, 0.f},
            {0.f, 0.f, 0.01f * 0.01f, 0.f},
            {0.f, 0.f, 0.f, 0.02f * 0.02f}
        }};
    }

public:
    void update(float voltage, float current, float temperature)
    {
        predict(current, temperature);
        correct(voltage, current, temperature);

        x.at(0) = std::clamp(x.at(0), 0.0f, 1.0f);
        x.at(4) = std::clamp(x.at(4), 0.1f, 0.5f);
    }

    void set(float soc)
    {
        x.at(0) =
            std::clamp(soc, 0.0f, 1.0f);
    }

    float get() const
    {
        return x.at(0);
    }

    float get_innovation() const
    {
        return __innovation;
    }

    float get_k_soc() const
    {
        return __k_soc;
    }

    float get_v_model() const
    {
        return __v_model;
    }

    float get_r0_i() const
    {
        return __r0_i;
    }

    float get_v1() const
    {
        return x.at(1);
    }

    float get_v2() const
    {
        return x.at(2);
    }

    float get_vh() const
    {
        return x.at(3);
    }

    float get_dr() const
    {
        return x.at(4);
    }
};

} // namespace PUTM
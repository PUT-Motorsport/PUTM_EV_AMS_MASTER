#pragma once

#include "main.h"

#include <utility>
#include <array>

/* macros for clarity, undef at the end of the file */
#define persistent  static inline constexpr
#define overridable static inline

namespace PUTM
{
    enum class CurrentDirection : int32_t
    {
        REVERSED = -1,
        FORWARD = 1
    };

    enum class InvalidTemperaturesStrategy : int32_t
    {
        NONE, 
        IGNORE_SELECTED,
        STATISTICAL_IMPLAUSIBILITY,
        // IgnoreSelectedFromEEPROM
    };

    enum class NTCType : size_t
    {
        _2_252K_3976K,      // GA2.2K3A1IA
        _2k_3560K,          // B57861S0202F040
        _10k_3434K,         // TG310J34GBNR
        _10k_3435K,         // 103AT-11, NTCLE413E2103F520L, NTCLE413E2103F106A
        _10k_3988K,         // B57861S0103J040
        _10k_3984K,         // NTCLE413E2103H400
        _10k_3977K,         // NTCLE400E3103H, TTS-10KC3-BZ, NTCLE300E3103SB
        _10k_3950K,         // PANE103395
    };

    namespace NTCPart
    {
        persistent NTCType DEFAULT             = NTCType::_10k_3435K;
        persistent NTCType GA2_2K3A1IA         = NTCType::_2_252K_3976K;
        persistent NTCType B57861S0202F040     = NTCType::_2k_3560K;
        persistent NTCType TG310J34GBNR        = NTCType::_10k_3434K;
        persistent NTCType _103AT_11           = NTCType::_10k_3435K;
        persistent NTCType NTCLE413E2103F520L  = NTCType::_10k_3435K;
        persistent NTCType NTCLE413E2103F106A  = NTCType::_10k_3435K;
        persistent NTCType B57861S0103J040     = NTCType::_10k_3988K;
        persistent NTCType NTCLE413E2103H400   = NTCType::_10k_3984K;
        persistent NTCType NTCLE400E3103H      = NTCType::_10k_3977K;
        persistent NTCType TTS_10KC3_BZ        = NTCType::_10k_3977K;
        persistent NTCType NTCLE300E3103SB     = NTCType::_10k_3977K;
        persistent NTCType PANE103395          = NTCType::_10k_3950K;
    };

    /**
     * @brief   Default config for most important stuff
     * @note    Beware that there is also config strored in simulated EEPROM
     *          and values strored there will take priority ( unless you perform full chip erase )
     */
    namespace CONFIG
    {
        /*==================================================================================================*/
        /*                                             CONFIG                                               */
        /*==================================================================================================*/

        /* Bq796xx stack size */
        persistent size_t STACK_SIZE { 12 };
        /* Cell count per bq796xx */
        persistent size_t CELL_COUNT_PER_DEVICE { 12 };
        /* Total cell count in stack */
        persistent size_t TOTAL_CELL_COUNT { STACK_SIZE * CELL_COUNT_PER_DEVICE };
        /* Temperatures count pre bq796xx */
        persistent size_t TEMPERATURES_COUNT_PER_DEVICE { 8 };
        /* Total temperatures count in stack */
        persistent size_t TOTAL_TEMPERATURES_COUNT { STACK_SIZE * TEMPERATURES_COUNT_PER_DEVICE };
        /* Cell max voltage in [mV] */
        persistent uint32_t CELL_MAX_VOLTAGE { 4200 };
        /* Cell min voltage in [mV] */
        persistent uint32_t CELL_MIN_VOLTAGE { 2500 };
        /* Cell min balancing voltage [mV] */
        persistent uint32_t CELL_MIN_BALANCING_VOLTAGE { 3500 };
        /* Cell overvoltage/undervoltage margin in [mV] */
        persistent uint32_t CELL_OVUV_MARGIN { 20 };
        /* Overvoltage trigger in [mV] */
        persistent uint32_t CELL_OV { CELL_MAX_VOLTAGE - CELL_OVUV_MARGIN };
        /* Undervoltage trigger in [mV] */
        persistent uint32_t CELL_UV { CELL_MIN_VOLTAGE + CELL_OVUV_MARGIN };
        /* Overtemperature trigger in [degC] */
        persistent float CELL_OT_FLOAT { 58.f };
        /* Undertemperature trigger in [degC] */
        persistent float CELL_UT_FLOAT { 0.f };
        /* [NOT USED] Max cell balancing time [ms] */
        // static constexpr uint32_t CELL_BALANCING_TIMEOUT { 15000 };Config::CELL_BALANCING_TIMEOUT
        /* Cell balance target [mV]*/
        persistent uint32_t CELL_BALANCE_TARGET { 5 };
        
        /* Voltage that is considered "High Voltage" according to FSG rules */
        persistent float MIN_HV_THRESH { 60.f };
        /* Voltage as a percentage of max voltage till which car caps should charged, a value between 0 and 1 */
        static constexpr float CAR_CHARGE_THRESH { 0.95f };
        /* Max current treshold for long timeout */
        persistent float MAX_CURRENT_THRESH_LONG { 150.f };
        /* Min current treshold for long timeout */
        persistent float MIN_CURRENT_THRESH_LONG { -150.f };
        /* Max current treshold for short timeout */
        persistent float MAX_CURRENT_THRESH_SHORT { 200.f };
        /* Min current treshold for short timeout */
        persistent float MIN_CURRENT_THRESH_SHORT { -200.f };

        /* Max voltage on battery in [V] */
        persistent float MAX_BAT_VOLTAGE { 599.2f }; // { std::min(TOTAL_CELL_COUNT * CELL_MAX_VOLTAGE / 1000.f, 600.f) };
        /* Max charging voltage in [V] */
        persistent float MAX_CHARGING_VOLTAGE { MAX_BAT_VOLTAGE };
        /*  */
        persistent float MAX_CHARGING_CURRENT { 14.f }; // [A] FIXME: change to real value
        /* Min voltage on battery */

        persistent float MIN_BAT_VOLTAGE { TOTAL_CELL_COUNT * CELL_MIN_VOLTAGE / 1000.f };

        /* Stack COM status poll interval in [ms] */
        // static constexpr uint32_t STACK_COM_STATUS_POLL_INTERVAL { 50 };
        /* Stack COM data poll interval in (data is polled one at a time every interval) [ms] */
        persistent uint32_t STACK_COM_DATA_POLL_INTERVAL { 4 };

        /* Stack timeout configuration */
        persistent uint32_t STACK_COM_TIMEOUT { 10 }; // [ms]

        /* Error check timeout, time after which an persistent error will be considered an true error */
        persistent uint32_t STANDARD_ERROR_TIMEOUT { 150 };

        /* Current error timeout, time after which an current error will be considered an true error */
        persistent uint32_t CURRENT_ERROR_LONG_TIMEOUT { 2000 };

        /**
         *  Precharge min waiting time expresed in [ms], if caps charge too slowly this shit will timeout,
         *  also used for min precharge wait, so the airs dont change states to fast
         */
        persistent uint32_t MIN_PRECHARGE_WAIT { 2000 };
        
        /* Precharge max wating time expresed in [ms], if caps charge too slowly this shit will timeout */
        persistent uint32_t MAX_PRECHARGE_WAIT { 10000 };

        /** 
         *	The air state machine starts in off state by default if it detects the charger it will exit the off 
	     *	state and enter the idle state
         */
        persistent uint32_t STATE_MACHINE_OFF_TO_IDLE_WAIT { 200 };
        /* Idle state hiccup wait time - airs cant be turned on faster than once per second */
        persistent uint32_t STATE_MACHINE_IDLE_TO_PRECHARGE_WAIT { 1000 };

        /* Which channel is used for car voltage measurement from 0 to 3*/
        persistent uint32_t CAR_VOLTAGE_CHANNEL { 3 };
        /* Which channel is used for accumulator voltage measurement from 0 to 3*/
        persistent uint32_t ACU_VOLTAGE_CHANNEL { 2 };
        /* Which channel is used to measure current */
        persistent uint32_t CURRENT_CHANNEL { 1 };
        /* Which channel is used to measure current reference */
        persistent uint32_t CURRENT_REF_CHANNEL { 0 };

        // TODO: topic for much later but maybe do a self offset/gain calibration
        persistent float CAR_VOLTAGE_OFFSET { 0.f };
        persistent float CAR_VOLTAGE_GAIN { -1000.f / 2 }; //idk needs more calibration
        persistent float ACU_VOLTAGE_OFFSET { 0.f };
        persistent float ACU_VOLTAGE_GAIN { -1000.f / 2 };
        persistent float CURRENT_OFFSET { -0.72f };
        /* Current measurement gain * 3 is for the resistor divider on the input */
        persistent float CURRENT_GAIN_NETWORK { -3.04744f }; // the inputs + & - are reversed on the hardware
        persistent float CURRENT_DIRECTION { static_cast<float>(CurrentDirection::REVERSED) };
        // static constexpr float CURRENT_GAIN { 1.f / (2.f / 300.f) };
        persistent float CURRENT_REF_OFFSET { 0.f };
        // static constexpr float CURRENT_REF_R1 { 9.98e+3f };
        // static constexpr float CURRENT_REF_R2 { 49.8e+3f };
        /* Calibrated for real value */
        persistent float CURRENT_REF_GAIN_NETWORK { 6.10565f };

        /* Quality of life */
        persistent float CELL_OV_FLOAT { static_cast<float>(CELL_OV) / 1000.f };
        persistent float CELL_UV_FLOAT { static_cast<float>(CELL_UV) / 1000.f };

        /* Logger buffers */
        persistent size_t ERROR_LOGGER_SIZE { 2 * 1024U };
        persistent size_t EVENT_LOGGER_SIZE { 2 * 1024U };

        /* JSON size buffer */
        persistent size_t TX_JSON_BUFFER_SIZE { 12 * 1024U };
        persistent size_t RX_JSON_BUFFER_SIZE { 1024U };
        persistent size_t RX_UART_BUFFER_SIZE { 128U };

        /* Thread buffers */
        persistent size_t TEST_THREAD_POOL_SIZE { 2048U };
        persistent size_t ADS131m04_THREAD_POOL_SIZE { 2048U };  
        persistent size_t BQ796xx_THREAD_POOL_SIZE { 2048U }; 
        persistent size_t MAIN_THREAD_POOL_SIZE { 4096U };
        persistent size_t USB_COM_THREAD_POOL_SIZE { 16 * 1024U };
        persistent size_t CAR_CAN_THREAD_POOL_SIZE { 2 * 1024U };
        persistent size_t CHARGER_CAN_THREAD_POOL_SIZE { 1024U };

        /* VUSB Treshold */
        persistent uint16_t USB_VBUS_THRESH { 500 };

        /* Cell nominal capacity in Ah */
        persistent float CELL_NOMINAL_CAPACITY { 4.97f * 3.f };
        persistent float CELL_NOMINAL_INTERNAL_RESISTANCE { 8e-3f / 3.f };
        // static constexpr float CELL_INTERNAL_RESISTANCE { 0.005f };
        persistent float A1 { 1.99458f }; 
        persistent float A2 { -0.99458f };
        persistent float C1 { 9.21999e-06f };
        persistent float C2 { -9.21879e-06f }; 
        persistent float D1 { CELL_NOMINAL_INTERNAL_RESISTANCE }; // cell internal resistance
        // TODO: make it 'tick' depended in the future in the SoC lib 
        persistent float DT { 0.05 }; // Filter udpate time

        overridable InvalidTemperaturesStrategy IGNORE_TEMPERATURES_STRATEGY { InvalidTemperaturesStrategy::STATISTICAL_IMPLAUSIBILITY };

        /**
         * @brief   Ignore temperatures by defineing pairs { device, measurement }, the expected format is device = 1..STACK_SIZE 
         *          and measurement 1..TEMPERATURES_COUNT_PER_DEVICE. If the format is not followed or outside of range the data 
         *          will be silently discarded
         */
        persistent std::array IGNORE_TEMPERATURES { std::to_array<std::pair<size_t, size_t>>({ 
            { }
            // { 1, 1 }, { 1, 2 },
            // { 2, 3 } // ex.
        })};

        overridable std::array IGNORE_TEMPERATURES_MATRIX = []() -> std::array<std::array<bool, TEMPERATURES_COUNT_PER_DEVICE>, STACK_SIZE>
        {
            using array = std::array<std::array<bool, TEMPERATURES_COUNT_PER_DEVICE>, STACK_SIZE>;
            array result { };
            for(auto &device : result) for(auto &temp : device) temp = false;

            for(auto ignored : IGNORE_TEMPERATURES)
            {
                if(ignored.first < 1 or ignored.first > STACK_SIZE or ignored.second < 1 or ignored.second > TEMPERATURES_COUNT_PER_DEVICE) continue;
                result[ignored.first - 1][ignored.second - 1] = true;
            }
            return result;
        }();

        /* used in StatisticalImplasubility case */
        persistent size_t MAX_IGNORABLE_TEMPERATURES { 3 };
        /* used in StatisticalImplasubility case */
        persistent float DEVIATION_THRESHOLD { 2.f };

        // static constexpr float POLYNOMIAL_OCV[] { 2034.7852020f, -9878.314180f, 20304.286795f, -22998.124140f, 15652.018744f, -6548.995145f, 1657.8141810f, -240.64692800f, 18.231580000f,  3.143621f };
        // static constexpr float POLYNOMIAL_OCV[] { -3.157435e+02, 1.313878e+03, -2.245027e+03, 2.032497e+03, -1.052418e+03, 3.165986e+02, -5.425675e+01, 5.359099e+00, 3.369238e+00 };
        persistent float POLYNOMIAL_OCV[] { -5.93254e+02, 4.12604e+03, -1.20854e+04, 1.96446e+04, -1.95357e+04, 1.23088e+04, -4.91288e+03, 1.20917e+03, -1.73966e+02, 1.38394e+01, 3.00371e+00 };
        // static constexpr float POLYNOMIAL_T_R[] { 2.828902e+02, 2.818811e-02, -1.131373e-05, 3.281373e-09, -6.383295e-13, 8.324239e-17, -7.271016e-21, 4.187227e-25, -1.522588e-29, 3.163823e-34, -2.860511e-39 };
        //static constexpr float POLYNOMIAL_T_R[] { 2.828902e+02, 2.818811e-02, -1.131373e-05, 3.281373e-09, -6.383295e-13, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        // polynomial for T(R) where T is in K and R is in kOhm

        /* NTC Polynomials for differnet R25 and Beta */
        namespace POLYNOMIAL_T_R
        {
            template<NTCType NTC_TYPE> persistent float
            COEFFS { };
            /* GA2.2K3A1IA */
            template<> inline constexpr float 
            COEFFS<NTCType::_2_252K_3976K>[] { -4.881202e-08f, 4.323928e-06f, -1.702507e-04f, 3.926696e-03f, -5.885218e-02f, 6.010298e-01f, -4.264790e+00f, 2.104563e+01f, -7.112485e+01f, 1.592053e+02f, -2.231069e+02f, 1.818063e+02f, -9.492483e+01f, 3.481740e+02f };
            /* B57861S0202F040 */
            template<> inline constexpr float 
            COEFFS<NTCType::_2k_3560K>[] { -2.125821e-06f, 1.419731e-04f, -4.227395e-03f, 7.400932e-02f, -8.458512e-01f, 6.625221e+00f, -3.632116e+01f, 1.398028e+02f, -3.732012e+02f, 6.711336e+02f, -7.724725e+02f, 5.262877e+02f, -2.076472e+02f, 3.632253e+02f };
            /* TG310J34GBNR */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3434K>[] { -3.056000e-15f, 9.749135e-13f, -1.388445e-10f, 1.164409e-08f, -6.386712e-07f, 2.406124e-05f, -6.362115e-04f, 1.185118e-02f, -1.537671e-01f, 1.351472e+00f, -7.656713e+00f, 2.587359e+01f, -5.005285e+01f, 3.698467e+02f };
            /* 103AT-11, NTCLE413E2103F520L, NTCLE413E2103F106A */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3435K>[] { -3.043398e-15f, 9.712310e-13f, -1.383666e-10f, 1.160776e-08f, -6.368741e-07f, 2.400045e-05f, -6.347724e-04f, 1.182718e-02f, -1.534867e-01f, 1.349220e+00f, -7.644731e+00f, 2.583415e+01f, -4.998199e+01f, 3.697906e+02f };
            /* B57861S0103J040 */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3988K>[] { -1.743565e-16f, 6.891902e-14f, -1.210786e-11f, 1.245906e-09f, -8.330127e-08f, 3.794500e-06f, -1.200731e-04f, 2.641760e-03f, -3.979163e-02f, 3.967988e-01f, -2.475833e+00f, 8.981255e+00f, -2.097755e+01f, 3.478779e+02f };
            /* NTCLE413E2103H400 */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3984K>[] { -1.784885e-16f, 7.043766e-14f, -1.235485e-11f, 1.269324e-09f, -8.473678e-08f, 3.854149e-06f, -1.217867e-04f, 2.675858e-03f, -4.025549e-02f, 4.009893e-01f, -2.499739e+00f, 9.060358e+00f, -2.110972e+01f, 3.479759e+02f };
            /* NTCLE400E3103H, TTS-10KC3-BZ, NTCLE300E3103SB */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3977K>[] { -1.784885e-16f, 7.043766e-14f, -1.235485e-11f, 1.269324e-09f, -8.473678e-08f, 3.854149e-06f, -1.217867e-04f, 2.675858e-03f, -4.025549e-02f, 4.009893e-01f, -2.499739e+00f, 9.060358e+00f, -2.110972e+01f, 3.479759e+02f };
            /* PANE103395 */
            template<> inline constexpr float 
            COEFFS<NTCType::_10k_3950K>[] { -2.175458e-16f, 8.467486e-14f, -1.465161e-11f, 1.485351e-09f, -9.787582e-08f, 4.395970e-06f, -1.372390e-04f, 2.981229e-03f, -4.438363e-02f, 4.380832e-01f, -2.710520e+00f, 9.756518e+00f, -2.227413e+01f, 3.488382e+02f };
        };

        /* Enable mapping packs to different polynomials */
        persistent bool ENABLE_NTC_MAPPING { true };
        //  
        persistent std::array<NTCType, STACK_SIZE> PACK_POLYNOMIAL_MAP { NTCPart::DEFAULT }; // ex.

        /**
         *      R1
         *      +---- Vout
         *      NTC
         */
        persistent float NOMINAL_R1 { 10.f };
        /* nominal voltage accross the resistor divider network for bq series is 5V */
        persistent float NOMINAL_TSREF { 5.f };
        
        /* FIR filter coeficients */
        persistent double _50HZ_RECT_FIR_COEFFS[] { -0.029720319698735946, 0.000000000000000020, 0.036324835187343932, -0.000000000000000020, -0.046703359526585060, 0.000000000000000020, 0.065384703337219074, -0.000000000000000020, -0.108974505562031795, 0.000000000000000020, 0.326923516686095372, 0.513530259153388680, 0.326923516686095372, 0.000000000000000020, -0.108974505562031795, -0.000000000000000020, 0.065384703337219074, 0.000000000000000020, -0.046703359526585060, -0.000000000000000020, 0.036324835187343932, 0.000000000000000020, -0.029720319698735946 };
        persistent double _2HZ_HAMMING_FIR_COEFFS[] { 0.000576110880220521, 0.000646822448048742, 0.000746173886528511, 0.000880485495847452, 0.001055865380883305, 0.001278093218249943, 0.001552506461458987, 0.001883890946839809, 0.002276377817745797, 0.002733348606047448, 0.003257350199003677, 0.003850021277918203, 0.004512031644686462, 0.005243035656095933, 0.006041640766708672, 0.006905391942899444, 0.007830772457077449, 0.008813221306509297, 0.009847167229931069, 0.010926079021878313, 0.012042531574028448, 0.013188286809477480, 0.014354388424299893, 0.015531269115327082, 0.016708868757928562, 0.017876761806466711, 0.019024292026397268, 0.020140712533626107, 0.021215329016118584, 0.022237643946748514, 0.023197499566239548, 0.024085217421448386, 0.024891732287200149, 0.025608718378814715, 0.026228705876144665, 0.026745185926567137, 0.027152702471551313, 0.027446929446248967, 0.027624732130639769, 0.027684211680294977, 0.027624732130639772, 0.027446929446248970, 0.027152702471551313, 0.026745185926567140, 0.026228705876144665, 0.025608718378814715, 0.024891732287200156, 0.024085217421448389, 0.023197499566239552, 0.022237643946748514, 0.021215329016118580, 0.020140712533626110, 0.019024292026397261, 0.017876761806466707, 0.016708868757928565, 0.015531269115327082, 0.014354388424299898, 0.013188286809477487, 0.012042531574028448, 0.010926079021878319, 0.009847167229931078, 0.008813221306509298, 0.007830772457077454, 0.006905391942899445, 0.006041640766708676, 0.005243035656095938, 0.004512031644686464, 0.003850021277918208, 0.003257350199003678, 0.002733348606047450, 0.002276377817745797, 0.001883890946839810, 0.001552506461458987, 0.001278093218249943, 0.001055865380883304, 0.000880485495847452, 0.000746173886528511, 0.000646822448048742, 0.000576110880220521 };
    
        /*==================================================================================================*/
        /*                                           DEV CONFIG                                             */
        /*==================================================================================================*/

        /* Turns off all errors */
        [[maybe_unused]] overridable bool TURN_OFF_ERROR_CHECKER { false };
        [[maybe_unused]] overridable bool TURN_OFF_TEMP_ERRORS { false };
        [[maybe_unused]] overridable bool TURN_OFF_CELL_VOLTAGE_ERRORS { false };
        [[maybe_unused]] overridable bool TURN_OFF_TS_ERRORS { false };
        [[maybe_unused]] overridable bool TURN_OFF_CURRENT_ERRORS { false };
        [[maybe_unused]] overridable bool MASK_CELL_VOLTAGE_ERRORS { false };
        [[maybe_unused]] overridable bool MASK_CELL_TEMPERATURE_ERRORS { false };
        [[maybe_unused]] overridable bool MASK_CAR_VOLTAGE_ERRORS { false };
        [[maybe_unused]] overridable bool MASK_ACU_VOLTAGE_ERRORS { false };
    };
}

/* TODO: implement config checker */

#undef persistent
#undef overridable
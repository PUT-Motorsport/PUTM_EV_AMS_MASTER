#pragma once

#include "main.h"

#include "atomic"

// #define DEBUG_IGNORE_CURRENT_ERRORS
// #define DEBUG_IGNORE_BAT_VOLTAGE_ERRORS

namespace PUTM
{
    namespace Config
    {
        /* DEV CONFIG */
        static constexpr bool TURN_OFF_ERRORS { false };

        /* Bq796xx stack size */
        static constexpr size_t STACK_SIZE { 12 };
        /* Cell count per bq796xx */
        static constexpr size_t CELL_COUNT_PER_DEVICE { 12 };
        /* Total cell count in stack */
        static constexpr size_t TOTAL_CELL_COUNT { STACK_SIZE * CELL_COUNT_PER_DEVICE };
        /* Temperatures count pre bq796xx */
        static constexpr size_t TEMPERATURES_COUNT_PER_DEVICE { 8 };
        /* Total temperatures count in stack */
        static constexpr size_t TOTAL_TEMPERATURES_COUNT { STACK_SIZE * TEMPERATURES_COUNT_PER_DEVICE };
        /* Cell max voltage in [mV] */
        static constexpr uint32_t CELL_MAX_VOLTAGE { 4200 };
        /* Cell min voltage in [mV] */
        static constexpr uint32_t CELL_MIN_VOLTAGE { 2500 };
        /* Cell min balancing voltage [mV] */
        static constexpr uint32_t CELL_MIN_BALANCING_VOLTAGE { 3500 };
        /* Cell overvoltage/undervoltage margin in [mV] */
        static constexpr uint32_t CELL_OVUV_MARGIN { 20 };
        /* Overvoltage trigger in [mV] */
        static constexpr uint32_t CELL_OV { CELL_MAX_VOLTAGE - CELL_OVUV_MARGIN };
        /* Undervoltage trigger in [mV] */
        static constexpr uint32_t CELL_UV { CELL_MIN_VOLTAGE + CELL_OVUV_MARGIN };
        /* Overtemperature trigger in [degC] */
        static constexpr float CELL_OT_FLOAT { 58.f };
        /* Undertemperature trigger in [degC] */
        static constexpr float CELL_UT_FLOAT { 0.f };
        /* Max cell balancing time */
        
        /* Voltage that is considered "High Voltage" according to FSG rules */
        static constexpr float MIN_HV_THRESH { 60.f };
        /* Voltage as a percentage of max voltage till which car caps should charged, a value between 0 and 1 */
        static constexpr float CAR_CHARGE_THRESH { 0.95f };
#if defined(DEBUG_IGNORE_CURRENT_ERRORS)
        /* Max current treshold for long timeout */
        static constexpr float MAX_CURRENT_THRESH_LONG { 1000.f };
        /* Min current treshold for long timeout */
        static constexpr float MIN_CURRENT_THRESH_LONG { -1000.f };
        /* Max current treshold for short timeout */
        static constexpr float MAX_CURRENT_THRESH_SHORT { 1000.f };
        /* Min current treshold for short timeout */
        static constexpr float MIN_CURRENT_THRESH_SHORT { -1000.f };
#else
        /* Max current treshold for long timeout */
        static constexpr float MAX_CURRENT_THRESH_LONG { 150.f };
        /* Min current treshold for long timeout */
        static constexpr float MIN_CURRENT_THRESH_LONG { -50.f };
        /* Max current treshold for short timeout */
        static constexpr float MAX_CURRENT_THRESH_SHORT { 200.f };
        /* Min current treshold for short timeout */
        static constexpr float MIN_CURRENT_THRESH_SHORT { -50.f };
#endif /* DEBUG_IGNORE_CURRENT_ERRORS */
        /* Max voltage on battery in [V] */
        static constexpr float MAX_BAT_VOLTAGE { std::min(TOTAL_CELL_COUNT * CELL_MAX_VOLTAGE / 1000.f, 600.f) };
        /* Max charging voltage in [V] */
        static constexpr float MAX_CHARGING_VOLTAGE { MAX_BAT_VOLTAGE };
        /*  */
        static constexpr float MAX_CHARGING_CURRENT { 12.f }; // [A] FIXME: change to real value
        /* Min voltage on battery */
#ifdef DEBUG_IGNORE_BAT_VOLTAGE_ERRORS
        static constexpr float MIN_BAT_VOLTAGE { -100.f };
#else
        static constexpr float MIN_BAT_VOLTAGE { TOTAL_CELL_COUNT * CELL_MIN_VOLTAGE / 1000.f };
#endif /* DEBUG_IGNORE_BAT_VOLTAGE_ERRORS */

        /* Stack COM status poll interval in [ms] */
        // static constexpr uint32_t STACK_COM_STATUS_POLL_INTERVAL { 50 };
        /* Stack COM data poll interval in (data is polled one at a time every interval) [ms] */
        static constexpr uint32_t STACK_COM_DATA_POLL_INTERVAL { 4 };

        /* Stack timeout configuration */
        static constexpr uint32_t STACK_COM_TIMEOUT { 10 }; // [ms]

        /* Error check timeout, time after which an persistent error will be considered an true error */
        static constexpr uint32_t STANDARD_ERROR_TIMEOUT { 150 };

        /* Current error timeout, time after which an current error will be considered an true error */
        static constexpr uint32_t CURRENT_ERROR_LONG_TIMEOUT { 2000 };

        /**
         *	Precharge min waiting time expresed in [ms], if caps charge too slowly this shit will timeout,
         *	also used for min precharge wait, so the airs dont change states to fast
         */
        static constexpr uint32_t MIN_PRECHARGE_WAIT { 2000 };
        
        /* Precharge max wating time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t MAX_PRECHARGE_WAIT { 10000 };

        /** 
         *	The air state machine starts in off state by default if it detects the charger it will exit the off 
	 *	state and enter the idle state
         */
        static constexpr uint32_t STATE_MACHINE_OFF_TO_IDLE_WAIT { 200 };
        /**
         *	Idle state hiccup wait time - airs cant be turned on faster than once per second
         */
        static constexpr uint32_t STATE_MACHINE_IDLE_TO_PRECHARGE_WAIT { 1000 };

        /* Which channel is used for car voltage measurement from 0 to s3*/
        static constexpr uint32_t CAR_VOLTAGE_CHANNEL { 3 };
        /* Which channel is used for accumulator voltage measurement from 0 to 3*/
        static constexpr uint32_t ACU_VOLTAGE_CHANNEL { 2 };
        /* Which channel is used to measure current */
        static constexpr uint32_t CURRENT_CHANNEL { 1 };
        static constexpr uint32_t CURRENT_REF_CHANNEL { 0 };

        // TODO: topic for much later but maybe do a self offset/gain calibration
        static constexpr float CAR_VOLTAGE_OFFSET { 0.f };
        static constexpr float CAR_VOLTAGE_GAIN { -1000.f / 2 }; //idk needs more calibration
        static constexpr float ACU_VOLTAGE_OFFSET { 0.f };
        static constexpr float ACU_VOLTAGE_GAIN { -1000.f / 2 };
        static constexpr float CURRENT_OFFSET { 0.43f };
        /* Current measurement gain * 3 is for the resistor divider on the input */
        static constexpr float CURRENT_GAIN_NETWORK { 3.04744f }; //{ 3.f };
        // static constexpr float CURRENT_GAIN { 1.f / (2.f / 300.f) };
        static constexpr float CURRENT_REF_OFFSET { 0.f };
        // static constexpr float CURRENT_REF_R1 { 9.98e+3f };
        // static constexpr float CURRENT_REF_R2 { 49.8e+3f };
        /* Calibrated for real value */
        static constexpr float CURRENT_REF_GAIN_NETWORK { 6.10565f };

        /* Quality of life */
        static constexpr float CELL_OV_FLOAT { static_cast<float>(CELL_OV) / 1000.f };
        static constexpr float CELL_UV_FLOAT { static_cast<float>(CELL_UV) / 1000.f };

        /* JSON size buffer */
        static constexpr uint32_t TX_JSON_BUFFER_SIZE { 8 * 1024U };
        static constexpr uint32_t RX_JSON_BUFFER_SIZE { 1024U };
        static constexpr uint32_t RX_UART_BUFFER_SIZE { 128U };

        /* VUSB Treshold */
        static constexpr uint16_t USB_VBUS_THRESH { 500 };

        /* Cell nominal capacity in Ah */
        static constexpr float CELL_NOMINAL_CAPACITY { 4.97f * 3.f };
        static constexpr float CELL_NOMINAL_INTERNAL_RESISTANCE { 8e-3f / 3.f };
        // static constexpr float CELL_INTERNAL_RESISTANCE { 0.005f };
        static constexpr float A1 { 1.99458f }; 
        static constexpr float A2 { -0.99458f };
        static constexpr float C1 { 9.21999e-06f };
        static constexpr float C2 { -9.21879e-06f }; 
        static constexpr float D1 { CELL_NOMINAL_INTERNAL_RESISTANCE }; // cell internal resistance
        // TODO: make it 'tick' depended in the future in the SoC lib 
        static constexpr float DT { 0.05 }; // Filter udpate time


        // static constexpr float POLYNOMIAL_OCV[] { 2034.7852020f, -9878.314180f, 20304.286795f, -22998.124140f, 15652.018744f, -6548.995145f, 1657.8141810f, -240.64692800f, 18.231580000f,  3.143621f };
        // static constexpr float POLYNOMIAL_OCV[] { -3.157435e+02, 1.313878e+03, -2.245027e+03, 2.032497e+03, -1.052418e+03, 3.165986e+02, -5.425675e+01, 5.359099e+00, 3.369238e+00 };
        static constexpr float POLYNOMIAL_OCV[] { -5.93254e+02, 4.12604e+03, -1.20854e+04, 1.96446e+04, -1.95357e+04, 1.23088e+04, -4.91288e+03, 1.20917e+03, -1.73966e+02, 1.38394e+01, 3.00371e+00 };
        // static constexpr float POLYNOMIAL_T_R[] { 2.828902e+02, 2.818811e-02, -1.131373e-05, 3.281373e-09, -6.383295e-13, 8.324239e-17, -7.271016e-21, 4.187227e-25, -1.522588e-29, 3.163823e-34, -2.860511e-39 };
        //static constexpr float POLYNOMIAL_T_R[] { 2.828902e+02, 2.818811e-02, -1.131373e-05, 3.281373e-09, -6.383295e-13, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        // polynomial for T(R) where T is in K and R is in kOhm
        static constexpr float POLYNOMIAL_T_R[] { 8.476916e-07, -7.580322e-05, 2.849150e-03, -5.860359e-02, 7.206923e-01, -5.455243e+00, 2.546881e+01, -7.461018e+01, 4.303188e+02 };
        // nominal ntc resistance (at 24degC) in kOhm
        static constexpr float NOMINAL_NTC_RESISTANCE { 10 };
        // nominal voltage accross the resistor divider network for bq series its 5V
        static constexpr float NOMINAL_TSREF { 5 };

        static constexpr double _50HZ_RECT_FIR_COEFFS[] { -0.029720319698735946, 0.000000000000000020, 0.036324835187343932, -0.000000000000000020, -0.046703359526585060, 0.000000000000000020, 0.065384703337219074, -0.000000000000000020, -0.108974505562031795, 0.000000000000000020, 0.326923516686095372, 0.513530259153388680, 0.326923516686095372, 0.000000000000000020, -0.108974505562031795, -0.000000000000000020, 0.065384703337219074, 0.000000000000000020, -0.046703359526585060, -0.000000000000000020, 0.036324835187343932, 0.000000000000000020, -0.029720319698735946 };
        static constexpr double _2HZ_HAMMING_FIR_COEFFS[] { 0.000576110880220521, 0.000646822448048742, 0.000746173886528511, 0.000880485495847452, 0.001055865380883305, 0.001278093218249943, 0.001552506461458987, 0.001883890946839809, 0.002276377817745797, 0.002733348606047448, 0.003257350199003677, 0.003850021277918203, 0.004512031644686462, 0.005243035656095933, 0.006041640766708672, 0.006905391942899444, 0.007830772457077449, 0.008813221306509297, 0.009847167229931069, 0.010926079021878313, 0.012042531574028448, 0.013188286809477480, 0.014354388424299893, 0.015531269115327082, 0.016708868757928562, 0.017876761806466711, 0.019024292026397268, 0.020140712533626107, 0.021215329016118584, 0.022237643946748514, 0.023197499566239548, 0.024085217421448386, 0.024891732287200149, 0.025608718378814715, 0.026228705876144665, 0.026745185926567137, 0.027152702471551313, 0.027446929446248967, 0.027624732130639769, 0.027684211680294977, 0.027624732130639772, 0.027446929446248970, 0.027152702471551313, 0.026745185926567140, 0.026228705876144665, 0.025608718378814715, 0.024891732287200156, 0.024085217421448389, 0.023197499566239552, 0.022237643946748514, 0.021215329016118580, 0.020140712533626110, 0.019024292026397261, 0.017876761806466707, 0.016708868757928565, 0.015531269115327082, 0.014354388424299898, 0.013188286809477487, 0.012042531574028448, 0.010926079021878319, 0.009847167229931078, 0.008813221306509298, 0.007830772457077454, 0.006905391942899445, 0.006041640766708676, 0.005243035656095938, 0.004512031644686464, 0.003850021277918208, 0.003257350199003678, 0.002733348606047450, 0.002276377817745797, 0.001883890946839810, 0.001552506461458987, 0.001278093218249943, 0.001055865380883304, 0.000880485495847452, 0.000746173886528511, 0.000646822448048742, 0.000576110880220521 };
}
}

/* TODO: implement config checker */
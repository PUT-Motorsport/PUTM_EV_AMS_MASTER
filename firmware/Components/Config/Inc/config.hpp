#pragma once

#include "main.h"

#include "atomic"

namespace PUTM
{
    namespace Config
    {
        /* Bq796xx stack size */
        static constexpr size_t STACK_SIZE { 10 };
        /* Cell count per bq796xx */
        static constexpr size_t CELL_COUNT_PER_DEVICE { 14 };
        /* Total cell count in stack */
        static constexpr size_t TOTAL_CELL_COUNT { STACK_SIZE * CELL_COUNT_PER_DEVICE };
        /* Temperatures count pre bq796xx */
        static constexpr size_t TEMPERATURES_COUNT_PER_DEVICE { 7 };
        /* Total temperatures count in stack */
        static constexpr size_t TOTAL_TEMPERATURES_COUNT { STACK_SIZE * TEMPERATURES_COUNT_PER_DEVICE };
        /* Cell max voltage in [mV] */
        static constexpr uint32_t CELL_MAX_VOLTAGE { 4200 };
        /* Cell min voltage in [mV] */
        static constexpr uint32_t CELL_MIN_VOLTAGE { 3000 };
        /* Overvoltage trigger in [mV] */
        static constexpr uint32_t CELL_OV { CELL_MAX_VOLTAGE - 50 };
        /* Undervoltage trigger in [mV] */
        static constexpr uint32_t CELL_UV { CELL_MIN_VOLTAGE + 50 };
        /* Overtemperature trigger in [degC] */
        static constexpr float CELL_OT { 100.f };
        /* Undertemperature trigger in [degC] */
        static constexpr float CELL_UT { -100.f };
        
        /* Voltage that is considered "High Voltage" according to FSG rules */
        static constexpr float MIN_HV_THRESH { 60.f };
        /* Voltage as a percentage of max voltage till which car caps should charged, a value between 0 and 1 */
        static constexpr float CAR_CHARGE_THRESH { 0.95f };
        /* Max current treshold for long timeout */
        static constexpr float MAX_CURRENT_THRESH_LONG { 150.f };
        /* Min current treshold for long timeout */
        static constexpr float MIN_CURRENT_THRESH_LONG { -50.f };
        /* Max current treshold for short timeout */
        static constexpr float MAX_CURRENT_THRESH_SHORT { 200.f };
        /* Min current treshold for short timeout */
        static constexpr float MIN_CURRENT_THRESH_SHORT { -MAX_CURRENT_THRESH_LONG };

        // FIXME: this is a temporary value, change it to real when done with tests
        /* Max voltage on battery in [V] */
        static constexpr float MAX_BAT_VOLTAGE { TOTAL_CELL_COUNT * CELL_MAX_VOLTAGE / 1000.f };
        /* Max charging voltage in [V] */
        static constexpr float CHARGING_VOLTAGE { TOTAL_CELL_COUNT * CELL_OV / 1000.f };
        // FIXME: this is a temporary value, change it to real when done with tests
        /* Min voltage on battery */
#ifdef DEBUG_TEST_MODE_1
        static constexpr float MIN_BAT_VOLTAGE { 200.f };
#else
        static constexpr float MIN_BAT_VOLTAGE { 450.f };
#endif /* DEBUG_TEST_MODE_1 */

        /* Stack COM status poll interval in [ms] */
        static constexpr uint32_t STACK_COM_STATUS_POLL_INTERVAL { 50 };
        /* Stack COM data poll interval in (data is polled one at a time every interval) [ms] */
        static constexpr uint32_t STACK_COM_DATA_POLL_INTERVAL { 100 };

        /* Stack timeout configuration */
        static constexpr uint32_t STACK_COM_TIMEOUT { 20 };

        // FIXME: maybe shorten the timeout on the board so the soft timeout can be lenghthen
        /* Error check timeout, time after which an persistent error will be considered an true error */
        static constexpr uint32_t STANDARD_ERROR_TIMEOUT { 200 };

        /* Current error timeout, time after which an current error will be considered an true error */
        static constexpr uint32_t CURRENT_ERROR_LONG_TIMEOUT { 2000 };

        // FIXME: for test i changed it to 4000ms change it back to 250ms when done with tests
        /* Precharge min waiting time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t MIN_PRECHARGE_WAIT { 2000 };
        
        /* Precharge max wating time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t MAX_PRECHARGE_WAIT { 6000 };

        /* Which channel is used for car voltage measurement from 0 to 3*/
        static constexpr uint32_t CAR_VOLTAGE_CHANNEL { 2 };
        /* Which channel is used for accumulator voltage measurement from 0 to 3*/
        static constexpr uint32_t ACU_VOLTAGE_CHANNEL { 1 };
        /* Which channel is used to measure current */
        static constexpr uint32_t CURRENT_CHANNEL { 0 };

        // FIXME: topic for much later but maybe do a self offset/gain calibration
        static constexpr float CAR_VOLTAGE_OFFSET { 0.f };
        static constexpr float CAR_VOLTAGE_GAIN { -1000.f / 2 }; //idk needs more calibration
        static constexpr float ACU_VOLTAGE_OFFSET { 0.f };
        static constexpr float ACU_VOLTAGE_GAIN { -1000.f / 2 };
        static constexpr float CURRENT_OFFSET { 371.25 };
        /* Current measurement gain * 3 is for the resistor divider on the input */
        static constexpr float CURRENT_GAIN { 1.f / (2.f / 300.f) * 3.f };

        /* Cell nominal capacity */
        static constexpr float CELL_NOMINAL_CAPACITY { 7.25439f };

        /* Quality of life */
        static constexpr float CELL_OV_FLOAT { static_cast<float>(CELL_OV) / 1000.f };
        static constexpr float CELL_UV_FLOAT { static_cast<float>(CELL_UV) / 1000.f };

        /* JSON size buffer */
        static constexpr uint32_t JSON_BUFFER_SIZE { 2048 };

        static constexpr float POLYNOMIAL_OCV[] { 2034.7852020f, -9878.314180f, 20304.286795f, -22998.124140f, 15652.018744f, -6548.995145f, 1657.8141810f, -240.64692800f, 18.231580000f,  3.143621f };
    }
}

/* TODO: implement config checker */
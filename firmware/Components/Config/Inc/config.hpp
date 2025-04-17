#pragma once

#include "main.h"

#include "atomic"

namespace PUTM
{
    namespace Config
    {
        /* Stack size - bq79616 in series excluding bq79600 */
        static constexpr size_t STACK_SIZE { 1 };
        /* Cell count per bq796xx */
        static constexpr size_t CELL_COUNT { 14 };
        /* Temperatures count pre bq796xx */
        static constexpr size_t TEMPERATURES_COUNT { 7 };
        /* Overvoltage trigger in [mV] */
        static constexpr uint32_t CELL_OV { 4000 };
        /* Undervoltage trigger in [mV] */
        static constexpr uint32_t CELL_UV { 3000 };
        
        /* Voltage that is considered "High Voltage" according to FSG rules */
        static constexpr float MIN_HV_THRESH { 60.f };
        /* Voltage as a percentage of max voltage till which car caps should charged, a value between 0 and 1 */
        static constexpr float CAR_CHARGE_THRESH { 0.95f };
        /* Max current treshold */
        static constexpr float MAX_CURRENT_THRESH { 150.f };
        /* Min current treshold */
        static constexpr float MIN_CURRENT_THRESH { -50.f };

        // FIXME: this is a temporary value, change it to real when done with tests
        /* Max voltage on battery */
        static constexpr float MAX_BAT_VOLTAGE { 600.f };
        // FIXME: this is a temporary value, change it to real when done with tests
        /* Min voltage on battery */
        static constexpr float MIN_BAT_VOLTAGE { 50.f };

        // FIXME: for test i changed it to 4000ms change it back to 250ms when done with tests
        /* Precharge min waiting time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t MIN_PRECHARGE_WAIT { 4000 };
        
        /* Precharge max wating time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t MAX_PRECHARGE_WAIT { 4500 };

        // FIXME: maybe shorten the timeout on the board so the soft timeout can be lenghthen
        /* Error check timeout, time after which an persistent error will be considered an true error */
        static constexpr uint32_t ERROR_TIMEOUT { 200 };

        /* Which channel is used for car voltage measurement from 0 to 3*/
        static constexpr uint32_t CAR_VOLTAGE_CHANNEL { 1 };
        /* Which channel is used for accumulator voltage measurement from 0 to 3*/
        static constexpr uint32_t ACU_VOLTAGE_CHANNEL { 0 };

        // FIXME: topic for much later but maybe do a self offset/gain calibration
        static constexpr float CAR_VOLTAGE_OFFSET { 0.f };
        static constexpr float CAR_VOLTAGE_GAIN { -205.570292 };
        static constexpr float ACU_VOLTAGE_OFFSET { 0.f };
        static constexpr float ACU_VOLTAGE_GAIN { 205.570292 };
    }
}

/* TODO: implement config checker */
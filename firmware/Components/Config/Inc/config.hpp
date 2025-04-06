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
        static constexpr float HV_THRESH { 60.f };
        /* Voltage as a percentage of max voltage till which car caps should charged, a value between 0 and 1 */
        static constexpr float CAR_CHARGE_THRESH { 0.95f };

        // FIXME: for test i changed it to 4000ms change it back to 250ms when done with tests
        /* Precharge min waiting time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t PRECHARGE_MIN_WAIT { 4000 };
        
        /* Precharge max wating time expresed in [ms], if caps charge too slowly this shit will timeout */
        static constexpr uint32_t PRECHARGE_MAX_WAIT { 4500 };

        /* Delay between packet sends */
        static constexpr uint32_t USB_TX_DELAY { 200 };
    }
}
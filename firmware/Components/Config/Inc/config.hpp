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

        /* Delay between packet sends */
        static constexpr uint32_t USB_TX_DELAY { 200 };
    }
}
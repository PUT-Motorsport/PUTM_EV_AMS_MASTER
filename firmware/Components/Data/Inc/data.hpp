#pragma once

#include "main.h"
#include "tx_api.h"

#include "config.hpp"

#include "atomic"
#include "array"

namespace PUTM
{
    struct Data
    {
        float current { 0.f };
        float acu_voltage { 0.f };
        float car_voltage { 0.f };
        float soc { 0.f };
        float cell_voltages[Config::STACK_SIZE][Config::CELL_COUNT] { 0.f };
        float cell_temperatures[Config::STACK_SIZE][Config::TEMPERATURES_COUNT] { 0.f };

        /* generic error flag it should be raised if error condition was found */
        bool error { false };
        /* if tsms voltage is present */
        bool tsms { false };
        /* if any cmd_on was received */
        bool cmd_on { false };
        /* if ams is plugged to charger */
        bool on_charger { false };

#ifdef TEST_MODE_1
        /* reset state machine */
        bool reset_state_machine { false };
#endif /* TEST_MODE_1 */
    };
}

extern PUTM::Data data;
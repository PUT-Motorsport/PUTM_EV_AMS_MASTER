#pragma once

#include "main.h"
#include "tx_api.h"

#include "config.hpp"

#include "atomic"
#include "array"

namespace PUTM
{
    //FIXME: heh its probably time to wory about race conditions
    struct Data
    {
        float current { 0.f };
        float acu_voltage { 0.f };
        float car_voltage { 0.f };
        float soc { 0.f };
        float cell_voltages[Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE] { 0.f };
        float cell_temperatures[Config::STACK_SIZE * Config::TEMPERATURES_COUNT_PER_DEVICE] { 0.f };
        bool cell_ovuv[Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE] { false };
        bool cell_otut[Config::STACK_SIZE * Config::TEMPERATURES_COUNT_PER_DEVICE] { false };
        float cell_max_voltage { 0.f };
        float cell_avg_voltage { 0.f };
        float cell_min_voltage { 0.f };
        float cell_max_temperature { 0.f };
        float cell_avg_temperature { 0.f };
        float cell_min_temperature { 0.f };

        float charging_current { 1.f };

        /* generic error flag it should be raised if error condition was found */
        bool error { false };
        /* if tsms voltage is present */
        bool tsms { false };
        /* if any cmd_hv was received */
        bool cmd_hv { false };
        /* if any cmd_off was received */
        bool cmd_charger { false };
        /* if ams is plugged to charger */
        bool on_charger { false };
        /* internal state tracking precharge */
        bool precharge { false };
        /* internal state tracking hv on */
        bool hv_on { false };
        /* Precharge error */
        uint32_t precharge_error { 0 };

#ifdef TEST_MODE_1
        /* reset state machine */
        bool reset_state_machine { false };
#endif /* TEST_MODE_1 */
    };
}

extern PUTM::Data data;
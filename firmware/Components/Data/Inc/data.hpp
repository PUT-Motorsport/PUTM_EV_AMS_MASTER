#pragma once

#include "main.h"
#include "tx_api.h"

#include "config.hpp"

#include "atomic"
#include "array"

namespace PUTM
{
    namespace Data
    {
        static float current { 0.f };
        static float acu_voltage { 0.f };
        static float car_voltage { 0.f };
        static float soc { 0.f };
        static float cell_voltages[Config::STACK_SIZE][Config::CELL_COUNT] { 0.f };
        static float cell_temperatures[Config::STACK_SIZE][Config::TEMPERATURES_COUNT] { 0.f };

        /* generic error flag it should be raised if error condition was found */
        static bool error;
    }
}
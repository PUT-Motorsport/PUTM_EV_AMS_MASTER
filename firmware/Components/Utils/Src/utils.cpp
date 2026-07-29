#include "utils.hpp"
#include "tx_api.h"
#include "cstring"



float Utils::UpdatesCounter::update(uint32_t tick)
{
    uint32_t dt = tick - last_update;
    last_update = tick;
    
    sum -= last_values[index];
    last_values[index] = dt;
    sum += dt;

    index++;
    if(index >= HISTORY_COUNT) index = 0;

    float mean = (float)sum / (float)HISTORY_COUNT;
    float ups = 1000.f / mean;

    return ups;
}

std::string_view Utils::get_error_name(HAL_StatusTypeDef error_code)
{
    switch(error_code)
    {
        case HAL_OK: return { "HAL_OK" };
        case HAL_ERROR: return { "HAL_ERROR" };
        case HAL_BUSY: return { "HAL_BUSY" };
        case HAL_TIMEOUT: return { "HAL_TIMEOUT" };
        default: return { "UNKNOWN_ERROR" };
    }
}
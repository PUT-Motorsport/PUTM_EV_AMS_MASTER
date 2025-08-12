#include "utils.hpp"
#include "tx_api.h"
#include "cstring"

float Utils::UpdatesCounter::update(uint32_t tick)
{
    float ups = 1000.f / (float)(tick - last_update);// * 0.001;
    last_update = tick;


    for(size_t i = 1; i < HISTORY_COUNT; i++) last_values[i] = last_values[i - 1];
    last_values[0] = ups;

    float sum { 0.f };
    for(size_t i = 0; i < HISTORY_COUNT; i++) sum += last_values[i];
    
    return sum / (float)HISTORY_COUNT;
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
#include "spi.h"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "ads131m04.hpp"
#include "tx_api.h"

using namespace PUTM;

Ads131m04::Device adc(&hspi1);

VOID ads131m04_thread_entry(__unused ULONG thread_input)
{
    uint32_t last_update { 0 };

    adc.init();
    adc.reset();
    tx_thread_sleep(20);

    data.ads_init_done = true;

    while(true)
    {
        adc.update();
        data.acu_voltage = adc.adc[Config::ACU_VOLTAGE_CHANNEL] * Config::ACU_VOLTAGE_GAIN;
        data.car_voltage = adc.adc[Config::CAR_VOLTAGE_CHANNEL] * Config::CAR_VOLTAGE_GAIN;
        data.current = adc.adc[Config::CURRENT_CHANNEL] * Config::CURRENT_GAIN - Config::CURRENT_OFFSET;
        
        data.update_times.ads_data_update_time = tx_time_get() - last_update;
        last_update = tx_time_get();
        tx_thread_sleep(20);
    }
}
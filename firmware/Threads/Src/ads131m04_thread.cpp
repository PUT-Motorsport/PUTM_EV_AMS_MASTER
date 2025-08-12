#include "spi.h"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "ads131m04.hpp"
#include "tx_api.h"
#include "utils.hpp"

using namespace PUTM;
using namespace Utils;



VOID ads131m04_thread_entry(__unused ULONG thread_input)
{
    static Ads131m04::Device adc(&hspi1);
    static UpdatesCounter updates;

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
        
        data.update_times.ads_updates_per_sec = updates.update(tx_time_get());
        tx_thread_sleep(10);
    }
}
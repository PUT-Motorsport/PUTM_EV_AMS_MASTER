#include "spi.h"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "ads131m04.hpp"

using namespace PUTM;

Ads131m04::Device adc(&hspi1);

VOID ads131m04_thread_entry(__unused ULONG thread_input)
{
    adc.init();
    adc.reset();

    while(true)
    {
        adc.update();
        data.acu_voltage = adc.adc[Config::ACU_VOLTAGE_CHANNEL] * Config::ACU_VOLTAGE_GAIN;
        data.car_voltage = adc.adc[Config::CAR_VOLTAGE_CHANNEL] * Config::CAR_VOLTAGE_GAIN;
        //data.current
        tx_thread_sleep(100);
    }
}
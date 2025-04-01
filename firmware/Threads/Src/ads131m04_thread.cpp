#include "spi.h"

#include "threads.hpp"
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
        tx_thread_sleep(100);
    }
}
#include "threads.hpp"
#include "bq796xx.hpp"

#include "usart.h"

using namespace PUTM;

Bq796xx::Device<1> bq(&huart4);

VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    bq.init();
    bq.init_stack();
    bq.init_voltage_measurement();
    bq.init_ovuv(3050, 4300);

    while(true)
    {
        bq.update_voltages();
		bq.update_status();
        tx_thread_sleep(100);
    }
}
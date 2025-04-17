#include "usart.h"

#include "bq796xx.hpp"
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"

using namespace PUTM;

Bq796xx::Device<1> bq(&huart4);

VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    bq.init();
    bq.init_uart();
    bq.init_stack();
    bq.init_voltage_measurement();
    bq.init_ovuv(3050, 4300);

    while(true)
    {
        bq.update_data();
		bq.update_status();

        /* copy data */
        for(size_t ic = 0; ic < Config::STACK_SIZE; ic++)
        {
            for(size_t cell = 0; cell < Config::CELL_COUNT; cell++)
            {
                data.cell_voltages[ic][cell] = bq.stack_device_data[ic].voltages[cell];
            }
        }

        tx_thread_sleep(100);
    }
}
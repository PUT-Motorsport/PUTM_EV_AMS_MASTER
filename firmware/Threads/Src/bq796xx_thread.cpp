#include "main.h"
#include "stm32h573xx.h"
#include "stm32h5xx_hal_def.h"
#include "tx_api.h"
#include "usart.h"
#include "algorithm"
#include "numeric"

#include "bq796xx.hpp"
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "wrapper/uart.hpp"
#include <cstdint>

using namespace PUTM;
using namespace Utils;



VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    static Uart uart3(&huart3);
    static Bq796xx::Device bq(&uart3);
    static UpdatesCounter updates;
    static uint32_t device_address { 1 }; 

    data.bq_init_status = bq.init();

    tx_thread_sleep(20);
    
    data.bq_init_status = bq.init();

    tx_thread_sleep(20);

    for(size_t i = 0; i < Config::STACK_SIZE; i++)
    {
        data.bq_read_data_status[device_address - 1] =  bq.read_single_data(data.cell_voltages[i],
                                                                            data.gpio_voltages[i],
                                                                            i + 1);
    }

    tx_thread_sleep(10);

    data.bq_init_done = true;

    uint32_t last_time = tx_time_get();

    while(true)
    {
        if(data.cmd_balancing_on)
        {
            data.cmd_balancing_on = false;
            bq.start_auto_balancing();
        }
        else if(data.cmd_balancing_off)
        {
            data.cmd_balancing_off = false;
            bq.stop_balancing();
        }
        uint32_t time = tx_time_get() - last_time;
        if(time > 1000 and not data.error)
        {
            last_time = tx_time_get();
            bq.set_balancing_voltage((uint32_t)(data.cell_min_voltage * 1000.f));
        }

        //TODO: add data read status handling and start strying to reset the unavaliable devices
        // maybe to sth simple like read till the last one avaliable start the single init
        // seqence from the N device and then jump to the first one

        data.bq_read_data_status[device_address - 1] =  bq.read_single_data(data.cell_voltages[device_address - 1],
                                                        data.gpio_voltages[device_address - 1],
                                                        device_address);
        
        device_address++;
        if(device_address > Config::STACK_SIZE) device_address = 1; 
        data.update_times.bq_updates_per_sec = updates.update(tx_time_get());
        
        tx_thread_sleep(Config::STACK_COM_DATA_POLL_INTERVAL);
    }
}
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

static constexpr float CELL_MIN_BALANCING_VOLTAGE_FLOAT { Config::CELL_MIN_BALANCING_VOLTAGE / 1000.f };

VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    static Uart uart3(&huart3);
    static Bq796xx::Device bq(&uart3);
    static UpdatesCounter updates;
    static uint32_t device_address { 1 }; 

    bool balancing_on = false;
    // bool balancing_cells[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { 0 };
    uint32_t last_balancing { 0 };
    size_t last_balancing_times[Config::STACK_SIZE] { 0 };
    float balancing_target { 5.f };

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
    while(true)
    {
        if(data.cmd_balancing_on)
        {
            data.cmd_balancing_on = false;
            balancing_on = true;
            balancing_target = data.cell_min_voltage;
        }
        else if(data.cmd_balancing_off)
        {
            data.cmd_balancing_off = false;
            balancing_on = false;
            bq.stop_balancing();
        }
        if(balancing_on and balancing_target < CELL_MIN_BALANCING_VOLTAGE_FLOAT)
        {
            data.warning = true;
            balancing_on = false;
            bq.stop_balancing();
        }
        // if(balancing_on)
        // {
        //     for(size_t i = 0; i < Config::CELL_COUNT_PER_DEVICE; i++)
        //     {
        //         if(balancing_cells[device_address- 1][i]) continue;
        //         if(data.cell_voltages[device_address - 1][i] < CELL_MIN_BALANCING_VOLTAGE_FLOAT)
        //         {
        //             data.warning = true;
        //             balancing_on = false;
        //             break;
        //         }
        //     }
        // }

        last_balancing = tx_time_get() - last_balancing_times[device_address - 1];

        if(last_balancing > 15000 and balancing_on)//and not data.error
        {
            size_t start_from = 0;
            if(data.cell_balancing[device_address - 1][0] == true) start_from = 1;
            for(size_t i = 0; i < Config::CELL_COUNT_PER_DEVICE; i++)
            {
                data.cell_balancing[device_address - 1][i] = false;
            }
            for(size_t i = start_from; i < Config::CELL_COUNT_PER_DEVICE; i += 2)
            {
                if(data.cell_voltages[device_address - 1][i] > balancing_target)
                data.cell_balancing[device_address - 1][i] = true;
            }

            last_balancing_times[device_address - 1] = tx_time_get();
            bq.set_balancing(data.cell_balancing[device_address - 1], device_address);
            bq.start_balancing(device_address);
        }
        //TODO: add data read status handling and start strying to reset the unavaliable devices
        // maybe to sth simple like read till the last one avaliable start the single init
        // seqence from the N device and then jump to the first one

        // if(balancing_on) bq.pause_balancing(device_address);

        data.bq_read_data_status[device_address - 1] =  bq.read_single_data(data.cell_voltages[device_address - 1],
                                                            data.gpio_voltages[device_address - 1],
                                                            device_address);
        // if(balancing_on) bq.resume_balancing(device_address);
        device_address++;
        if(device_address > Config::STACK_SIZE) device_address = 1; 
        data.update_times.bq_updates_per_sec = updates.update(tx_time_get());
        
        tx_thread_sleep(Config::STACK_COM_DATA_POLL_INTERVAL);
    }
}
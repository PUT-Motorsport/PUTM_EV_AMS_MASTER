#include "main.h"
#include "tx_api.h"
#include "usart.h"
#include "algorithm"
#include "numeric"

#include "bq796xx.hpp"
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "wrapper/uart.hpp"

using namespace PUTM;

Uart uart4(&huart4);

Bq796xx::Device bq(&uart4);

VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    // bq.init({ Config::CELL_OV, Config::CELL_UV }, { Config::CELL_OT, Config::CELL_UT });

    auto tmp = bq.init();

    uint32_t last_status_poll { 0 };
    uint32_t last_data_poll { 0 };
    uint32_t device_address { 1 };
    
    while(true)
    {
        if(tx_time_get() - last_status_poll > Config::STACK_COM_STATUS_POLL_INTERVAL)
        {      
            bq.read_stack_status(data.cell_ovuv,
                                 data.cell_otut);
            last_status_poll = tx_time_get();
        }
        if(tx_time_get() - last_data_poll > Config::STACK_COM_DATA_POLL_INTERVAL)
        {
            bq.read_single_data(data.cell_voltages[device_address - 1],
                                data.cell_temperatures[device_address - 1],
                                device_address);
            
            /* update max, min and avg */
            // data.cell_max_voltage = std::max(*std::max_element(std::begin(data.cell_voltages[device_address - 1]), 
            //                                                    std::end(data.cell_voltages[device_address - 1])), 
            //                                                    data.cell_max_voltage);
            // data.cell_min_voltage = std::min(*std::min_element(std::begin(data.cell_voltages[device_address - 1]), 
            //                                                    std::end(data.cell_voltages[device_address - 1])), 
            //                                                    data.cell_min_voltage);
            // data.cell_avg_voltage = std::accumulate(std::begin(data.cell_voltages), std::end(data.cell_voltages), 0.f) / (float)(Config::TOTAL_CELL_COUNT);
            // data.cell_max_temperature = std::max(*std::max_element(std::begin(data.cell_temperatures[device_address - 1]), 
            //                                                       std::end(data.cell_temperatures[device_address - 1])),
            //                                                       data.cell_avg_voltage);
            // data.cell_min_temperature = *std::min_element(std::begin(data.cell_temperatures[device_address - 1]), 
            //                                               std::end(data.cell_temperatures[device_address - 1]),
            //                                               data.cell_min_temperature);
            // data.cell_avg_temperature = std::accumulate(std::begin(data.cell_temperatures), std::end(data.cell_temperatures), 0.f) / (float)(Config::TOTAL_TEMPERATURES_COUNT);
            
            last_data_poll = tx_time_get();
            device_address++;
            if(device_address > Config::STACK_SIZE) device_address = 1; 
        }

        tx_thread_sleep(10);
    }
}
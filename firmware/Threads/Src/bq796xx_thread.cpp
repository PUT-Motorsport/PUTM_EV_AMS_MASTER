#include "main.h"
#include "tx_api.h"
#include "usart.h"
#include "algorithm"
#include "numeric"

#include "bq796xx.hpp"
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"

using namespace PUTM;

Bq796xx::Device bq(&huart4);

VOID bq796xx_thread_entry(__unused ULONG thread_input)
{
    bq.init();
    bq.init_uart();
    bq.init_stack();
    bq.init_voltage_measurement();
    bq.init_ovuv(3050, 4300);
    bq.init_temperature_measurements();
    bq.init_otut(70, 30);
    bq.start_measurements();

    while(true)
    {
        bq.update_data(data.cell_voltages,
                       data.cell_temperatures);
        bq.update_status(data.cell_ovuv,
                         data.cell_otut);

        /* update max, min and avg */
        data.cell_max_voltage = *std::max_element(std::begin(data.cell_voltages), std::end(data.cell_voltages));
        data.cell_min_voltage = *std::min_element(std::begin(data.cell_voltages), std::end(data.cell_voltages));
        data.cell_avg_voltage = std::accumulate(std::begin(data.cell_voltages), std::end(data.cell_voltages), 0.f) / (float)(Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE);
        data.cell_max_temperature = *std::max_element(std::begin(data.cell_temperatures), std::end(data.cell_temperatures));
        data.cell_min_temperature = *std::min_element(std::begin(data.cell_temperatures), std::end(data.cell_temperatures));
        data.cell_avg_temperature = std::accumulate(std::begin(data.cell_temperatures), std::end(data.cell_temperatures), 0.f) / (float)(Config::STACK_SIZE * Config::TEMPERATURES_COUNT_PER_DEVICE);

        tx_thread_sleep(50);
    }
}
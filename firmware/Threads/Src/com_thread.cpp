extern "C"
{
#include "main.h"
#include "usb.h"
#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_cdc_acm.h"
}
#include "cstring"
#include "cstdio"
#include "usart.h"
#include "algorithm"
#include "fdcan.h"

#include "ArduinoJson.h"
#include "can_interface.hpp"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "charger.hpp"
#include "wrapper/fdcan.hpp"
#include "state_machine.hpp"
#include "com/charger_state_machine.hpp"

using namespace PUTM;
using namespace PUTM::Config;

using namespace PUTM_CAN;

static StaticJsonDocument<512> json;

static constexpr ULONG tx_rx_buffer_size { 512 };

static CHAR tx_buffer[tx_rx_buffer_size];
static CHAR rx_buffer[tx_rx_buffer_size];

static ULONG rx_actual_size { 0 };
static ULONG tx_actual_size { 0 };

extern TX_MUTEX tx_buffer_mutex;
extern TX_MUTEX rx_buffer_mutex;

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

VOID car_can_thread_entry(__unused ULONG thread_input)
{
    start_can(&hfdcan2);

    while(true)
    {
        /* Send basic battery info on CAN */
        BMS_HV_main bms_hv_main
        {
            .voltage_sum = (uint16_t)(data.acu_voltage * 10.f),
            .current = (int16_t)(data.current * 10.f),
            .temp_max = 100,//(uint8_t)(data.cell_max_temperature * 10.f),
            .temp_avg = 100,//(uint8_t)(data.cell_avg_temperature * 10.f),
            .soc = 100,//(uint16_t)(data.soc * 10.f),
            .ok = true,//not data.error,
            .precharge = data.precharge
        };

        /* Send data to CAN */
        auto bms_hv_main_frame = PUTM_CAN::Can_tx_message(bms_hv_main, can_tx_header_BMS_HV_MAIN);
        auto status = bms_hv_main_frame.send(hfdcan2);
        if(status != HAL_StatusTypeDef::HAL_OK)
        {
            // Error_Handler();
        }

        if (PUTM_CAN::can.get_dashboard_new_data() && PUTM_CAN::can.get_dashboard().ts_activation_button)
		{
			data.cmd_hv = true;
		}

        tx_thread_sleep(20);
    }
}

extern StateMachine charger_state_machine;

VOID charger_can_thread_entry(__unused ULONG thread_input)
{
    start_can(&hfdcan1);

    init_charger_state_machine(&charger_state_machine);

    while(true)
    {
        charger_state_machine.update();
        tx_thread_sleep(200);
    }
}

/**
 *  @brief  USB communication thread entry point
 *  @note   Usb communication is realized using an external USB<->UART converter
 */
VOID usb_com_thread_entry(__unused ULONG thread_input)
{
    Uart uart(&huart1);
    uart.init();
    uart.set_baudrate(115200);

    ArduinoJson::StaticJsonDocument<Config::JSON_BUFFER_SIZE> json;

    while(true)
    {
        if(data.usb_connected)
        {
            json.clear();
            json["timestamp"] = tx_time_get();
            json["current"] = data.current;
            json["acc_voltage"] = data.acu_voltage;
            json["car_voltage"] = data.car_voltage;
            json["soc"] = data.soc;
            for(size_t i = 0; i < Config::TOTAL_CELL_COUNT; i++)
            {
                json["cell_voltages"][i] = data.cell_voltages[i];
            }
            for(size_t i 0 = 0; i < Config::TOTAL_TEMPERATURES_COUNT; i++)
            {
                json["cell_temperatures"][i] = data.cell_temperatures[i];
            }

            char buffer[JSON_BUFFER_SIZE] { };
            serializeJson(json, buffer, JSON_BUFFER_SIZE);
            uart.await_tx_dma((uint8_t *)buffer, strlen(buffer));
        }
        tx_thread_sleep(200);
    }
}

// /**
//  *  @brief  Usb rx thread
//  *  @note   The `ux_device_class_cdc_acm_read()` function works in a blocking mode until a transfer request froma host computer is
//  *          received 
//  */
// VOID usb_rx_thread_entry(__unused ULONG thread_input)
// {
//     while(true)
//     {
//         if(cdc_acm != UX_NULL)
//         {
//             ux_device_class_cdc_acm_read(cdc_acm, (UCHAR *)rx_buffer, tx_rx_buffer_size, &rx_actual_size);
//         }
//         tx_thread_sleep(20);
//     }
// }

// ULONG usbx_tx_status = 0;
// /**
//  *  @brief  Usb tx thread
//  *  @note   The `ux_device_class_cdc_acm_write()` function works in a blocking mode until a transfer request froma host computer is
//  *          received 
//  */
// VOID usb_tx_thread_entry(__unused ULONG thread_input)
// {
//     while(true)
//     {
//         if(cdc_acm != UX_NULL)
//         {
//             json["timestamp"] = tx_time_get();
//             json["current"] = data.current;
//             json["acu_voltage"] = data.acu_voltage;
//             json["car_voltage"] = data.car_voltage;
//             json["soc"] = data.soc;

//             // serializeJson(json, tx_buffer, tx_rx_buffer_size);
//             serializeJsonPretty(json, tx_buffer, tx_rx_buffer_size);
//             ux_device_class_cdc_acm_write(cdc_acm, (UCHAR *)(tx_buffer), strlen(tx_buffer), &tx_actual_size);
//         }
//         tx_thread_sleep(20);
//     }
// }
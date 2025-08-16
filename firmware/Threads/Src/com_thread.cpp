#include "error_checker.hpp"
#include "stm32h5xx_ll_adc.h"
#include <cstddef>
extern "C"
{
#include "main.h"
}
#include "cstring"
#include "cstdio"
#include "usart.h"
#include "algorithm"
#include "fdcan.h"
#include "adc.h"

#include "ArduinoJson.h"
#include "can_interface.hpp"

#include "utils.hpp"
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "charger.hpp"
#include "wrapper/fdcan.hpp"
#include "wrapper/uart.hpp"
#include "state_machine.hpp"
#include "com/charger_state_machine.hpp"
#include "logger.hpp"
#include "string_view"
#include "wrapper/gpio.hpp"

using namespace PUTM;
using namespace PUTM::Config;
using namespace Utils;

using namespace PUTM_CAN;

/* seperate thread for timing tweeks */
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
            .temp_max = (uint8_t)(data.cell_max_temperature), //(uint8_t)(data.cell_max_temperature * 10.f),
            .temp_avg = (uint8_t)(data.cell_avg_temperature), //(uint8_t)(data.cell_avg_temperature * 10.f),
            .soc = (uint16_t)(data.soc * 1000.f),
            .ok = not data.error,
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

extern Logger<2 * 1024> error_logger;

extern StateMachine air_state_machine;
extern StateMachine charger_state_machine;

extern ErrorChecker error_checker;

extern Gpio usb_reset;

// extern TX_MUTEX tx_buffer_mutex;
// extern TX_MUTEX rx_buffer_mutex;

static StaticJsonDocument<Config::TX_JSON_BUFFER_SIZE> tx_json;
static char tx_char_buffer[TX_JSON_BUFFER_SIZE] { }; // TODO: Maybe change to its own buffer size
static StaticJsonDocument<Config::RX_JSON_BUFFER_SIZE> rx_json;
static char rx_char_buffer[RX_UART_BUFFER_SIZE] { };

static Uart uart(&huart1);

static std::function<void(size_t)> usb_rx_callback = [](size_t size)
{
    if(size == 0) return; // No data received

    DeserializationError error = deserializeJson(rx_json, rx_char_buffer, RX_UART_BUFFER_SIZE);

    if(error)
    {
        // TODO: for now just set the warining flag
        data.warning = true;
    }
    else
    {
        std::string_view command = rx_json["command"];
        if(command == "charger_on")
        {
            data.cmd_charger = true;
        }
        else if(command == "charger_off")
        {
            data.cmd_charger = false;
        }
        else if(command == "set_current")
        {
            // if no value is provided ArduinoJson will assume 0.0f
            float charging_current = rx_json["value"];
            charging_current = std::clamp(charging_current, 0.0f, Config::MAX_CHARGING_CURRENT);
            data.charging_current = charging_current;
        }
        else if(command == "balancing_on")
        {
            data.cmd_balancing_on = true;
        }
        else if(command == "balancing_off")
        {
            data.cmd_balancing_off = true;
        }
        else if(command == "service_mode_on")
        {
            data.service_mode = true;
        }
        else if(command == "service_mode_off")
        {
            data.service_mode = false;
        }
        else if(command == "even_moar_data_on")
        {
            data.even_moar_data = true;
        }
        else if(command == "even_moar_data_off")
        {
            data.even_moar_data = false;
        }
        data.last_command = command;
    }

    uart.async_rx_unknown_dma(rx_char_buffer, RX_UART_BUFFER_SIZE, &usb_rx_callback);
};

/**
 *  @brief  USB communication thread entry point
 *  @note   Usb communication is realized using an external USB<->UART converter
 */
VOID usb_com_thread_entry(__unused ULONG thread_input)
{
    static bool first_run { true };

    // uart.init();
    uart.set_baudrate(250000);
    // uart.set_rx_timeout(10);
    uart.async_rx_unknown_dma(rx_char_buffer, RX_UART_BUFFER_SIZE, &usb_rx_callback);

    while(true)
    {
        if(not data.usb_connected) first_run = true;
        if(data.usb_connected) //if()
        {
            if(first_run)
            {
                first_run = false;
                usb_reset.set();
                continue;
            }
            else 
            {
                usb_reset.reset();
            }

            tx_json.clear();
            tx_json["timestamp"] = tx_time_get();
            tx_json["current"] = data.current;
            tx_json["acc_voltage"] = data.acu_voltage;
            tx_json["car_voltage"] = data.car_voltage;
            tx_json["soc"] = data.soc;
            tx_json["cell_max_voltage"] = data.cell_max_voltage;
            tx_json["cell_avg_voltage"] = data.cell_avg_voltage;
            tx_json["cell_min_voltage"] = data.cell_min_voltage;
            tx_json["cell_max_temperature"] = data.cell_max_temperature;
            tx_json["cell_avg_temperature"] = data.cell_avg_temperature;
            tx_json["cell_min_temperature"] = data.cell_min_temperature;
            tx_json["set_charging_current"] = data.charging_current;
            for(size_t i = 0; i < Config::TOTAL_CELL_COUNT; i++)
            {
                size_t idev = i / Config::CELL_COUNT_PER_DEVICE;
                size_t icell = i % Config::CELL_COUNT_PER_DEVICE;
                tx_json["cell_voltages"][idev][icell] = data.cell_voltages[idev][icell];
            }
            for(size_t i = 0; i < Config::TOTAL_TEMPERATURES_COUNT; i++)
            {
                size_t idev = i / Config::TEMPERATURES_COUNT_PER_DEVICE;
                size_t icell = i % Config::TEMPERATURES_COUNT_PER_DEVICE;
                tx_json["cell_temperatures"][idev][icell] = data.cell_temperatures[idev][icell];
            }
            tx_json["errors"] = JsonArray();
            for(auto error : error_checker)
            {
                tx_json["errors"].add(error.name);
            }
            tx_json["service_mode"] = data.service_mode;
            if(data.service_mode)
            {
                tx_json["usb_connected"] = data.usb_connected;
                tx_json["cmd_hv"] = data.cmd_hv;
                tx_json["cmd_charger"] = data.cmd_charger;
                tx_json["cmd_balancing_on"] = data.cmd_balancing_on;
                tx_json["cmd_balancing_off"] = data.cmd_balancing_off;
                tx_json["bq_init_status"] = get_error_name(data.bq_init_status);
                tx_json["error"] = data.error;
                tx_json["warning"] = data.warning;
                tx_json["on_charger"] = data.on_charger;
                tx_json["tsms"] = data.tsms;
                tx_json["precharge"] = data.precharge;
                tx_json["hv_on"] = data.hv_on;
                for(size_t i = 0; i < Config::TOTAL_CELL_COUNT; i++)
                {
                    size_t idev = i / Config::CELL_COUNT_PER_DEVICE;
                    size_t icell = i % Config::CELL_COUNT_PER_DEVICE;
                    tx_json["cell_socs"][idev][icell] = data.cell_socs[idev][icell].get();
                }
                for(size_t i = 0; i < Config::STACK_SIZE; i++)
                {
                    tx_json["bq_com_status"][i] = get_error_name(data.bq_read_data_status[i]);
                }
                for(size_t i = 0; i < Config::TOTAL_TEMPERATURES_COUNT; i++)
                {
                    size_t idev = i / Config::TEMPERATURES_COUNT_PER_DEVICE;
                    size_t icell = i % Config::TEMPERATURES_COUNT_PER_DEVICE;
                    tx_json["gpio_voltages"][idev][icell] = data.gpio_voltages[idev][icell];
                }
                tx_json["sm_air_state"] = air_state_machine.get_current_state_name();
                tx_json["sm_charger_state"] = charger_state_machine.get_current_state_name();
                tx_json["bq_updates_per_sec"] = data.update_times.bq_updates_per_sec;
                tx_json["ads_updates_per_sec"] = data.update_times.ads_updates_per_sec;
                tx_json["main_updates_per_sec"] = data.update_times.main_updates_per_sec;
                tx_json["logs"] = JsonArray();
                for(auto log : error_logger)
                {
                    tx_json["logs"].add(log);
                }
                if(data.even_moar_data)
                {
                    tx_json["vusb"] = data.vusb;
                    tx_json["usb_connected"] = data.usb_connected;
                }
            }

            // serializeJson(json, buffer, JSON_BUFFER_SIZE);
            serializeJson(tx_json, tx_char_buffer, TX_JSON_BUFFER_SIZE - 1);
            auto s = strlen(tx_char_buffer);
            //redundant newline at the end
            tx_char_buffer[s] = '\n';
            s += 1;
            if(uart.await_tx_dma((uint8_t *)tx_char_buffer, s, 500) != HAL_OK)
            {
                data.warning = true;
            }
        }
        tx_thread_sleep(200);
    }
}
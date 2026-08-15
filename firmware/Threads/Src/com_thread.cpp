#include "error_checker.hpp"
#include "stm32h5xx_hal_fdcan.h"
#include "stm32h5xx_ll_adc.h"
#include "tx_api.h"
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
#include "can_driver.hpp"
#include "PUTM_CAN_M.h"

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
using namespace Utils;

using namespace PUTM_CAN;

enum struct ComMode
{
    Normal,
    Kalman
};

ComMode com_mode { ComMode::Normal };

putm_ev_can::CanDriver can_driver;

extern StateMachine air_state_machine;
extern StateMachine charger_state_machine;

extern ErrorChecker error_checker;

extern Gpio usb_reset;

// extern TX_MUTEX tx_buffer_mutex;
// extern TX_MUTEX rx_buffer_mutex;

static StaticJsonDocument<CONFIG::TX_JSON_BUFFER_SIZE> tx_json;
static char tx_char_buffer[CONFIG::TX_JSON_BUFFER_SIZE] { }; // TODO: Maybe change to its own buffer size
static StaticJsonDocument<CONFIG::RX_JSON_BUFFER_SIZE> rx_json;
static char rx_char_buffer[CONFIG::RX_JSON_BUFFER_SIZE] { };

static Uart uart(&huart1);

static std::string_view command;

static char error_write_buffer[128];



/* seperate thread for timing tweeks */
VOID car_can_thread_entry(__unused ULONG thread_input)
{
    {
        FDCAN_FilterTypeDef filter_config
        {
            .IdType = FDCAN_STANDARD_ID,
            .FilterIndex = 0,
            .FilterType = FDCAN_FILTER_MASK,
            .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
            .FilterID1 = 0,
            .FilterID2 = 0
        };

        HAL_FDCAN_ConfigFilter(&hfdcan2, &filter_config);
        HAL_FDCAN_ConfigTxDelayCompensation(&hfdcan2, 9, 0);
        HAL_FDCAN_EnableTxDelayCompensation(&hfdcan2);
        // HAL_FDCAN_ConfigTimeoutCounter(&hfdcan2, uint32_t TimeoutOperation, uint32_t TimeoutPeriod)
    }

    /* Send basic battery info on CAN */

    if(!can_driver.Init(&hfdcan2))
    {
        data.warning = true;
        data.loggers.errors.log_error("CAN init failed");
        while(true) tx_thread_sleep(1000);
    }

    can_driver.RegisterCallback<PUTM_CAN_M_dashboard_t>(PUTM_CAN_M_DASHBOARD_FRAME_ID, [](const PUTM_CAN_M_dashboard_t& dashboard)
    {
        data.cmd_hv = dashboard.ts_activation_button;
    });

    while(true)
    {
        PUTM_CAN_M_bms_hv_main_t bms_hv_main
        {
            .voltage_sum = (uint16_t)(data.acu_voltage * 10.f),
            .current = (int16_t)(data.current * 10.f),
            .temp_max = (uint8_t)(data.cell_max_temperature), //(uint8_t)(data.cell_max_temperature * 10.f),
            .temp_avg = (uint8_t)(data.cell_avg_temperature), //(uint8_t)(data.cell_avg_temperature * 10.f),
            .soc = (uint16_t)(data.current_integral * 10.f),
            .ok = not data.error, 
            .precharge = data.precharge,
            .ts_on = data.hv_on
        };

        if(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) == 0)
        {
            uint32_t txFifoRequest = HAL_FDCAN_GetLatestTxFifoQRequestBuffer(&hfdcan2);
            if (HAL_FDCAN_IsTxBufferMessagePending(&hfdcan2, txFifoRequest))
            {
                HAL_FDCAN_AbortTxRequest(&hfdcan2, txFifoRequest);
            }
        }

        if(not can_driver.Send(PUTM_CAN_M_BMS_HV_MAIN_FRAME_ID, bms_hv_main))
        {
            data.warning = true;
        }

        tx_thread_sleep(20);
    }
}

static std::function<void(size_t)> usb_rx_callback = [](size_t size)
{
    DeserializationError error = deserializeJson(rx_json, rx_char_buffer, CONFIG::RX_UART_BUFFER_SIZE);

    if(error)
    {
        // data.loggers.events.log_event("RX ERR", tx_time_get());
        data.warning = true;
    }
    else
    {
        command = rx_json["command"];
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
            charging_current = std::clamp(charging_current, 0.0f, CONFIG::MAX_CHARGING_CURRENT);
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
        else if(command == "com_mode")
        {
            std::string_view mode = rx_json["value"];
            if(mode == "kalman")
            {
                com_mode = ComMode::Kalman;
            }
            else if(mode == "normal")
            {
                com_mode = ComMode::Normal;
            }
        }
        else if(command == "t_off")
        {
            size_t device = rx_json["device"];
            size_t temp = rx_json["temp"];
            if(0 < device and device <= CONFIG::STACK_SIZE and
                0 < temp and temp <= CONFIG::TEMPERATURES_COUNT_PER_DEVICE)
            {
                CONFIG::IGNORE_TEMPERATURES_MATRIX[device - 1][temp - 1] = true;
            }
        }
        else if(command == "t_on")
        {
            size_t device = rx_json["device"];
            size_t temp = rx_json["temp"];
            if(0 < device and device <= CONFIG::STACK_SIZE and
               0 < temp and temp <= CONFIG::TEMPERATURES_COUNT_PER_DEVICE)
            {
                CONFIG::IGNORE_TEMPERATURES_MATRIX[device - 1][temp - 1] = false;
            }
        }
        else if(command == "i_g")
        {
            float gain = rx_json["gain"];
            CONFIG::CURRENT_GAIN = gain;
        }
        else if(command == "i_b")
        {
            float bias = rx_json["bias"];
            CONFIG::CURRENT_BIAS = bias;
        }
        else if(command == "acu_g")
        {
            float gain = rx_json["gain"];
            CONFIG::ACU_VOLTAGE_GAIN = gain; 
        }
        else if(command == "car_g")
        {
            float gain = rx_json["gain"];
            CONFIG::CAR_VOLTAGE_GAIN = gain;
        }
        else if(command == "dr")
        {
            float value = rx_json["value"];
            CONFIG::CURRENT_BIAS = value;
        }
        else if(command == "save_cfg")
        {
            data.commands.save_config = true;
        }
        data.last_command = command;
    }

    uart.async_rx_unknown_dma(rx_char_buffer, CONFIG::RX_UART_BUFFER_SIZE, &usb_rx_callback);
};



/**
 *  @brief  USB communication thread entry point
 *  @note   Usb communication is realized using an external USB<->UART converter
 */
VOID usb_com_thread_entry(__unused ULONG thread_input)
{
    static bool first_run { true };

    // uart.init();
    uart.set_baudrate(460800);
    // uart.set_rx_timeout(10);

    while(true)
    {
        if(not data.usb_connected)
        {
            first_run = true;
        }
        if(data.usb_connected and first_run)
        {
            first_run = false;
            data.loggers.events.log_event("USB DET", tx_time_get());
            uart.abort_async_rx_unknown_dma();
            uart.async_rx_unknown_dma(rx_char_buffer, CONFIG::RX_UART_BUFFER_SIZE, &usb_rx_callback);
        }
        if(data.usb_connected)
        {
            tx_json.clear();
            tx_json["timestamp"] = tx_time_get();
            if(com_mode == ComMode::Normal)
            {
                tx_json["current"] = data.current;
                tx_json["current_reference"] = data.current_reference;
                tx_json["acc_voltage"] = data.acu_voltage;
                tx_json["car_voltage"] = data.car_voltage;
                tx_json["soc"] = data.soc;
                tx_json["current_integral"] = data.current_integral;
                tx_json["cell_max_voltage"] = data.cell_max_voltage;
                tx_json["cell_avg_voltage"] = data.cell_avg_voltage;
                tx_json["cell_min_voltage"] = data.cell_min_voltage;
                tx_json["cell_balance"] = (data.cell_max_voltage - data.cell_min_voltage);
                tx_json["cell_max_temperature"] = data.cell_max_temperature;
                tx_json["cell_avg_temperature"] = data.cell_avg_temperature;
                tx_json["cell_min_temperature"] = data.cell_min_temperature;
                tx_json["charging_current"] = data.charging_current;
                tx_thread_relinquish();
                for(size_t i = 0; i < CONFIG::TOTAL_CELL_COUNT; i++)
                {
                    size_t idev = i / CONFIG::CELL_COUNT_PER_DEVICE;
                    size_t icell = i % CONFIG::CELL_COUNT_PER_DEVICE;
                    tx_json["cell_voltages"][idev][icell] = data.cell_voltages[idev][icell];
                    
                    tx_thread_relinquish();
                }
                for(size_t i = 0; i < CONFIG::TOTAL_TEMPERATURES_COUNT; i++)
                {
                    size_t idev = i / CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                    size_t icell = i % CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                    tx_json["cell_temperatures"][idev][icell] = data.cell_temperatures[idev][icell];
                    tx_thread_relinquish();
                }
                for(size_t i = 0; i < CONFIG::TOTAL_CELL_COUNT; i++)
                {
                    size_t idev = i / CONFIG::CELL_COUNT_PER_DEVICE;
                    size_t icell = i % CONFIG::CELL_COUNT_PER_DEVICE;
                    tx_json["cell_balancing"][idev][icell] = data.cell_balancing[idev][icell];
                    tx_thread_relinquish();
                }
                for(size_t i = 0; i < CONFIG::TOTAL_TEMPERATURES_COUNT; i++)
                {
                    size_t idev = i / CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                    size_t icell = i % CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                    tx_json["kek"][idev][icell] = CONFIG::IGNORE_TEMPERATURES_MATRIX[idev][icell];
                    tx_thread_relinquish();
                }
                tx_json["errors"] = JsonArray();
                for(auto error : error_checker)
                {
                    tx_json["errors"].add(error.name);
                    tx_thread_relinquish();
                }
                tx_thread_relinquish();
                tx_json["service_mode"] = data.service_mode;
                //if(data.service_mode)
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
                    tx_thread_relinquish();
                    for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
                    {
                        tx_json["bq_com_status"][i] = get_error_name(data.bq_read_data_status[i]);
                        tx_thread_relinquish();
                    }
                    for(size_t i = 0; i < CONFIG::TOTAL_TEMPERATURES_COUNT; i++)
                    {
                        size_t idev = i / CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                        size_t icell = i % CONFIG::TEMPERATURES_COUNT_PER_DEVICE;
                        tx_json["gpio_voltages"][idev][icell] = data.gpio_voltages[idev][icell];
                        tx_thread_relinquish();
                    }
                    tx_json["sm_air_state"] = air_state_machine.get_current_state_name();
                    tx_json["sm_charger_state"] = charger_state_machine.get_current_state_name();
                    tx_json["bq_ups"] = data.update_times.bq;
                    tx_json["ads_ups"] = data.update_times.ads;
                    tx_json["main_ups"] = data.update_times.main;
                    tx_json["soc_ups"] = data.update_times.soc;
                    tx_json["checker_errors"] = JsonArray();
                    tx_thread_relinquish();
                    for(auto log : data.loggers.errors)
                    {
                        tx_json["checker_errors"].add(log);
                        tx_thread_relinquish();
                    }
                    tx_json["events"] = JsonArray();
                    for(auto log : data.loggers.events)
                    {
                        tx_json["events"].add(log);
                        tx_thread_relinquish();
                    }
                    //if(data.even_moar_data)
                    {
                        tx_json["vusb"] = data.vusb;
                        tx_json["usb_connected"] = data.usb_connected;
                        tx_json["last_command"] = data.last_command;
                    }
                    tx_thread_relinquish();
                    {
                        tx_json["charger_battery_read_voltage"] = data.charger.battery_read_voltage;
                        tx_json["charger_battery_read_current"] = data.charger.battery_read_current;
                        tx_json["charger_hardware_fail"] = data.charger.hardware_fail;
                        tx_json["charger_over_temperature"] = data.charger.over_temperature;
                        tx_json["charger_in_voltage_fail"] = data.charger.in_voltage_fail;
                        tx_json["charger_starting_state"] = data.charger.starting_state;
                        tx_json["charger_communication_state"] = data.charger.communication_state;
                        tx_json["charger_last_recive_tick"] = data.charger.last_recive_tick;
                    }
                    tx_thread_relinquish();
                }
            }
            if(com_mode == ComMode::Kalman)
            {
                tx_json["k0"]   = data.kalman.k_soc;
                tx_json["inn"]  = data.kalman.innovation;
                tx_json["v1"]   = data.kalman.v1;
                tx_json["v2"]   = data.kalman.v2;
                tx_json["vh"]   = data.kalman.vh;
                tx_json["dr"]   = data.kalman.dr;
                tx_json["vm"]   = data.kalman.v_model;
                tx_json["i"]    = data.current;
                tx_json["soc"]  = data.soc;
                tx_json["v"]    = data.acu_voltage;
            }

            // serializeJson(json, buffer, JSON_BUFFER_SIZE);
            serializeJson(tx_json, tx_char_buffer, CONFIG::TX_JSON_BUFFER_SIZE - 2);
            auto s = strlen(tx_char_buffer);
            //redundant newline at the end
            tx_char_buffer[s] = '\n';
            s += 1;
            if(uart.await_tx_dma((uint8_t *)tx_char_buffer, s, 2000) != HAL_OK)
            {
                data.warning = true;
            }
        }
        tx_thread_sleep(10);
    }
}
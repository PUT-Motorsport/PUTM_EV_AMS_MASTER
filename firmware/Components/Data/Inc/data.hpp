#pragma once

#include "main.h"
#include "stm32h5xx_hal_def.h"
#include "tx_api.h"

#include "config.hpp"
#include "soc.hpp"
#include "logger.hpp"

#include "atomic"
#include "array"
#include "string_view"

namespace PUTM
{
    struct Data
    {
        float current { 0.f };
        float current_reference { 0.f };
        float acu_voltage { 0.f };
        float car_voltage { 0.f };
        float soc { 0.f };
        float cell_voltages[CONFIG::STACK_SIZE][CONFIG::CELL_COUNT_PER_DEVICE] { 0.f };
        float gpio_voltages[CONFIG::STACK_SIZE][CONFIG::TEMPERATURES_COUNT_PER_DEVICE] { 0.f };
        float cell_temperatures[CONFIG::STACK_SIZE][CONFIG::TEMPERATURES_COUNT_PER_DEVICE] { 0.f };
        SoC cell_socs[CONFIG::STACK_SIZE][CONFIG::CELL_COUNT_PER_DEVICE] { };

        // float cell_socs[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { };

        bool cell_balancing[CONFIG::STACK_SIZE][CONFIG::CELL_COUNT_PER_DEVICE] { false };
        // bool cell_balancing[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { false };
        // bool cell_ovuv[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { false };
        // bool cell_otut[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE] { false };
        float cell_max_voltage { 0.f };
        float cell_avg_voltage { 0.f };
        float cell_min_voltage { 0.f };
        float cell_max_temperature { 0.f };
        float cell_avg_temperature { 0.f };
        float cell_min_temperature { 0.f };
        float cell_voltage_sum { 0.f };

        float charging_current { 0.0f };
        float current_integral { 0.0f };

        /* generic error flag it should be raised if error condition was found */
        bool error { false };
        /* generic warning flag */
        bool warning { false };
        /* if tsms voltage is present */
        bool tsms { false };
        /*  
         *  if any cmd_hv was received, this field is reset after enetring next state, or if 
         *  entring on (precharge state) wasnt possible. Always exectutes "turn off airs sequence if"
         *  if this command is received again and the airs are closed 
         */
        bool cmd_hv { false };
        /* if any cmd_off was received */
        bool cmd_charger { false };
        /* if ams is plugged to charger */
        bool on_charger { false };
        /* internal state tracking precharge */
        bool precharge { false };
        /* internal state tracking hv on */
        bool hv_on { false };
        /* if balancing command received */
        bool cmd_balancing_on { false };
        bool cmd_balancing_off { false };
        /* USB connected */
        bool usb_connected { false };
        /* */
        bool bq_init_done { false };
        /* */
        bool ads_init_done { false };
        /* system init done */
        bool system_init_done { false };
        /* service mode */
        bool service_mode { false };
        /* even moar data */
        bool even_moar_data { false };
        /* last command received */
        std::string_view last_command { };
        /* Precharge error */
        uint32_t precharge_error { 0 };
        /* VUSB */
        uint16_t vusb { 0 };
        /* bq init status */
        HAL_StatusTypeDef bq_init_status { HAL_OK };
        /* bq read data status */
        HAL_StatusTypeDef bq_read_data_status[CONFIG::STACK_SIZE] { HAL_OK };
        
        struct
        {
            bool on_charger { false };
            bool tsms { false };
            bool usb_connected { false }; 
        } gpio;

        struct
        {
            bool balancing_on { false };
            bool balancing_off { false };
            bool charger_on { false };
            bool charger_off { false };
        } commands;

        struct
        {
            float bq_updates_per_sec { 0.f };
            float ads_updates_per_sec { 0.f };
            float main_updates_per_sec { 0.f };
        } update_times;

        // TODO: figure out the bools what false / true means  
        struct
        {
            float battery_read_voltage { 0 };
            float battery_read_current { 0 };
            bool hardware_fail { false };
            bool over_temperature { false };
            bool in_voltage_fail { false };
            bool starting_state { false };
            bool communication_state { false };
            uint32_t last_recive_tick { 0 };
        } charger;

        struct
        {
            Logger<CONFIG::ERROR_LOGGER_SIZE> errors;
            Logger<CONFIG::EVENT_LOGGER_SIZE> events;
        } loggers;

#ifdef TEST_MODE_1
        /* reset state machine */
        bool reset_state_machine { false };
#endif /* TEST_MODE_1 */
    };
}

// need to name the data object more appropriately, but for now it is fine or make the data object a singleton, but for now it is fine

extern PUTM::Data data;
#pragma once

#include "main.h"
#include "stm32h5xx_hal_def.h"
#include "tx_api.h"

#include "config.hpp"
#include "soc.hpp"

#include "atomic"
#include "array"
#include "string_view"

namespace PUTM
{
    struct Data
    {
        float current { 0.f };
        float acu_voltage { 0.f };
        float car_voltage { 0.f };
        float soc { 0.f };
        float cell_voltages[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { 0.f };
        float gpio_voltages[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE] { 0.f };
        float cell_temperatures[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE] { 0.f };
        SoC cell_socs[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { };
        // bool cell_balancing[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { false };
        // bool cell_ovuv[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE] { false };
        // bool cell_otut[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE] { false };
        float cell_max_voltage { 0.f };
        float cell_avg_voltage { 0.f };
        float cell_min_voltage { 0.f };
        float cell_max_temperature { 0.f };
        float cell_avg_temperature { 0.f };
        float cell_min_temperature { 0.f };

        float charging_current { 0.0f };

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
        HAL_StatusTypeDef bq_read_data_status[Config::STACK_SIZE] { HAL_OK };
        
        struct
        {
            float bq_updates_per_sec { 0.f };
            float ads_updates_per_sec { 0.f };
            float main_updates_per_sec { 0.f };
        } update_times;

#ifdef TEST_MODE_1
        /* reset state machine */
        bool reset_state_machine { false };
#endif /* TEST_MODE_1 */
    };
}

extern PUTM::Data data;
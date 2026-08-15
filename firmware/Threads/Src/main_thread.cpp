#include "eeprom.hpp"
#include "main.h"
#include "stm32h573xx.h"
#include "stm32h5xx_hal_flash.h"
#include "stm32h5xx_ll_adc.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"
#include "adc.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "wrapper/fdcan.hpp"
#include "wrapper/uart.hpp"
#include "data.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "state_machine.hpp"
#include "main/air_state_machine.hpp"
#include "error_checker.hpp"
#include "main/error_checker.hpp" 
#include "com/charger_state_machine.hpp"
#include "charger.hpp"
#include "soc.hpp"
#include "polynomial.hpp"
#include "logger.hpp"
//FIXME: delete this later
#include "pchip.hpp"
// #include "eeprom.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <array>

using namespace PUTM;
using namespace Utils;

/* Wrappers */
Gpio led_err(LED_ERROR_GPIO_Port, LED_ERROR_Pin, true);
Gpio led_wrn(LED_WARNING_GPIO_Port, LED_WARNING_Pin, true);
Gpio led_ok(LED_OK_GPIO_Port, LED_OK_Pin, true);
Gpio sig_err(SIG_AMS_ERROR_GPIO_Port, SIG_AMS_ERROR_Pin, true);
Gpio sig_air_pre(SIG_AIR_PRE_GPIO_Port, SIG_AIR_PRE_Pin, false);
Gpio sig_air_p(SIG_AIR_P_GPIO_Port, SIG_AIR_P_Pin, false);
Gpio sig_air_m(SIG_AIR_M_GPIO_Port, SIG_AIR_M_Pin, false);
Gpio det_air_pre(DET_AIR_PRE_GPIO_Port, DET_AIR_PRE_Pin, false);
Gpio det_air_p(DET_AIR_P_GPIO_Port, DET_AIR_P_Pin, false);
Gpio det_air_m(DET_AIR_M_GPIO_Port, DET_AIR_M_Pin, false);
Gpio det_tsms(DET_TSMS_GPIO_Port, DET_TSMS_Pin, true);
Gpio det_charger(DET_CHARGER_GPIO_Port, DET_CHARGER_Pin, true);
Gpio usb_reset(USB_RESET_GPIO_Port, USB_RESET_Pin, true);
// Gpio adc_dry(ADC_NDRY_GPIO_Port, ADC_NDRY_Pin, true);
// Gpio bq_flt(NFLT_GPIO_Port, NFLT_Pin, true);

constexpr Polynomial t_r_poly { CONFIG::POLYNOMIAL_T_R::COEFFS<NTCPart::DEFAULT> }; //CONFIG::POLYNOMIAL_T_R::COEFFS<NTCType::_10k_3434K>

SoC soc;

ChargerCanRxController charger_rx;

extern StateMachine air_state_machine;
extern StateMachine charger_state_machine;  

extern ErrorChecker error_checker;

extern TX_TIMER soc_update_timer;

Eeprom<PUTM::EepromAddress> eeprom;

/**
 * @brief Calculate resistance of ntc from voltage
 * 
 * @param voltage voltage on resistor divider
 * @return constexpr float 
 */
float r_u_lambda(float voltage)
{
    return CONFIG::NOMINAL_R1 / (CONFIG::NOMINAL_TSREF - voltage) * voltage;
};

/**
 * @brief Entry point for the main thread of the system.
 *
 * This function initializes hardware interfaces, resets status LEDs and signals,
 * and waits for the completion of ADS and BQ device initialization. Once initialization
 * is complete, it sets up the state of charge (SoC) for each cell, marks the system as
 * initialized, and enters the main loop.
 *
 * In the main loop, the following operations are performed periodically:
 * - Toggles the "OK" LED to indicate the thread is alive.
 * - Reads GPIO inputs for TSMS and charger detection.
 * - Updates the SoC for each cell based on voltage, current, and charger status.
 * - Calculates the average SoC across all cells.
 * - Updates cell temperatures using voltage readings and a polynomial evaluation.
 * - Computes min, max, and average values for cell voltages and temperatures, ignoring
 *   values outside configured over/under thresholds.
 * - Updates the AIR state machine.
 * - Checks for system errors and logs unique errors if detected.
 * - Toggles the warning LED if a warning is present.
 * - Updates the main thread's update rate statistics.
 * - Sleeps for a configured period before repeating.
 *
 * @param thread_input Unused thread input parameter.
 */
VOID main_thread_entry(__unused ULONG thread_input)
{
    static Uart debug_uart(&huart4);
    static UpdatesCounter updates;

    led_wrn.reset();
    led_ok.reset();
    led_err.reset();
    sig_err.reset();
    sig_air_pre.set();
    sig_air_p.set();
    sig_air_m.set();

    init_air_state_machine(&air_state_machine);

    init_error_checker(&error_checker);

    HAL_FLASH_Unlock();
    if (eeprom.init() != HAL_OK) 
    {
        data.loggers.errors.log_error("EPPROM ERR");
    }
    auto temp_ign_0 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_0, 0);
    auto temp_ign_1 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_1, 0);
    auto temp_ign_2 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_2, 0);
    auto temp_ign_3 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_3, 0);
    auto temp_ign_4 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_4, 0);
    auto temp_ign_5 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_5, 0);
    auto temp_ign_6 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_6, 0);
    auto temp_ign_7 = eeprom.new_var<uint32_t>(EepromAddress::TEMP_IGNORE_7, 0);
    auto i_calib_gain   = eeprom.new_var<float>(EepromAddress::I_CALIB_GAIN,   CONFIG::CURRENT_GAIN);
    auto i_calib_bias   = eeprom.new_var<float>(EepromAddress::I_CALIB_BIAS,   CONFIG::CURRENT_BIAS);
    auto acu_calib_gain = eeprom.new_var<float>(EepromAddress::ACU_CALIB_GAIN, CONFIG::ACU_VOLTAGE_GAIN);
    auto acu_calib_bias = eeprom.new_var<float>(EepromAddress::ACU_CALIB_BIAS, CONFIG::ACU_VOLTAGE_BIAS);
    auto car_calib_gain = eeprom.new_var<float>(EepromAddress::CAR_CALIB_GAIN, CONFIG::CAR_VOLTAGE_GAIN);
    auto car_calib_bias = eeprom.new_var<float>(EepromAddress::CAR_CALIB_BIAS, CONFIG::CAR_VOLTAGE_BIAS);

    std::array<Eeprom<EepromAddress>::Variable<uint32_t>*, 8> temp_ign({
        &temp_ign_0, &temp_ign_1, &temp_ign_2, &temp_ign_3,
        &temp_ign_4, &temp_ign_5, &temp_ign_6, &temp_ign_7,
    });
    for(size_t device = 0; device < CONFIG::STACK_SIZE; device++)
    {
        for(size_t temp = 0; temp < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; temp++)
        {
            size_t index = device % 4;
            size_t offset = (device / 4) * 8 + temp;
            auto opt = temp_ign[index]->read();
            if(opt.has_value())
            {
                CONFIG::IGNORE_TEMPERATURES_MATRIX[device][temp] = opt.value() & (1 << offset);
            }
            else
            {
                CONFIG::IGNORE_TEMPERATURES_MATRIX[device][temp] = false;
            }
        }
    }
    if(auto opt = i_calib_gain.read(); opt.has_value())     CONFIG::CURRENT_GAIN = opt.value();
    if(auto opt = i_calib_bias.read(); opt.has_value())     CONFIG::CURRENT_BIAS = opt.value();
    if(auto opt = acu_calib_gain.read(); opt.has_value())   CONFIG::ACU_VOLTAGE_GAIN = opt.value();
    if(auto opt = acu_calib_bias.read(); opt.has_value())   CONFIG::ACU_VOLTAGE_BIAS = opt.value();
    if(auto opt = car_calib_gain.read(); opt.has_value())   CONFIG::CAR_VOLTAGE_GAIN = opt.value();
    if(auto opt = car_calib_bias.read(); opt.has_value())   CONFIG::CAR_VOLTAGE_BIAS = opt.value();
    HAL_FLASH_Lock();

    data.flash_init_done = true;

    while(not (data.ads_init_done and data.bq_init_done))
    {
        tx_thread_sleep(10);
    }
#ifndef  DEBUG
    tx_timer_activate(&soc_update_timer);
#endif
    HAL_ADC_Start(&hadc1);

    data.tsms = det_tsms.read();
    data.on_charger = det_charger.read();

    usb_reset.reset();

    data.system_init_done = true;
    data.loggers.events.log_event("SYS INIT DONE", tx_time_get());

    while(true)
    {
        /* Is alive */
        led_ok.toggle();

        /* Read gpios */
        data.tsms = det_tsms.read();
        data.on_charger = det_charger.read();

        /* update true temps */
        if constexpr (CONFIG::ENABLE_NTC_MAPPING)
        {
            static_for<0, CONFIG::STACK_SIZE>([&]<int DEVICE>()
            {
                constexpr Polynomial t_r_poly { CONFIG::POLYNOMIAL_T_R::COEFFS<CONFIG::PACK_POLYNOMIAL_MAP[DEVICE]> };
                for(size_t chanel = 0; chanel < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; chanel++)
                {
                    float r = r_u_lambda(data.gpio_voltages[DEVICE][chanel]);
                    data.cell_temperatures[DEVICE][chanel] = t_r_poly.evaluate(r) - 273.15f;
                }
            });
        }
        else
        {
            for(size_t device = 0; device < CONFIG::STACK_SIZE; device++)
            {
                for(size_t chanel = 0; chanel < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; chanel++)
                {
                    auto r = r_u_lambda(data.gpio_voltages[device][chanel]);
                    data.cell_temperatures[device][chanel] = t_r_poly.evaluate(r) - 273.15f;
                }
            }
        }
        
        if(CONFIG::IGNORE_TEMPERATURES_STRATEGY == PUTM::InvalidTemperaturesStrategy::IGNORE_SELECTED)
        {
            for(size_t device = 0; device < CONFIG::STACK_SIZE; device++)
            {
                for(size_t chanel = 0; chanel < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; chanel++)
                {
                    if(CONFIG::IGNORE_TEMPERATURES_MATRIX[device][chanel])
                    {
                        data.cell_temperatures[device][chanel] = 0.f;
                    }
                }
            }
        }
        /* update min max */
        float max_voltage = 0.0f;
        float min_voltage = 100.f;
        float max_temperature = 0.0f;
        float min_temperature = 100.0f;
        float accumulator_voltage = 0.0f;
        float accumulator_temperature = 0.0f;
        size_t acc_temp_offset = 0;
        for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < CONFIG::CELL_COUNT_PER_DEVICE; j++)
            {
                accumulator_voltage += data.cell_voltages[i][j];
            }
            for(size_t j = 0; j < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                accumulator_temperature += data.cell_temperatures[i][j];
            }
        }

        data.cell_avg_voltage = accumulator_voltage / (CONFIG::STACK_SIZE * CONFIG::CELL_COUNT_PER_DEVICE);
        data.cell_avg_temperature = accumulator_temperature / (CONFIG::STACK_SIZE * CONFIG::TEMPERATURES_COUNT_PER_DEVICE - acc_temp_offset);
        data.cell_voltage_sum = accumulator_voltage;

        if(CONFIG::IGNORE_TEMPERATURES_STRATEGY == InvalidTemperaturesStrategy::STATISTICAL_IMPLAUSIBILITY)
        {
            float squared_difference_sum = 0.f;

            for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
            {
                for(size_t j = 0; j < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; j++)
                {
                    const float difference = data.cell_temperatures[i][j] - data.cell_avg_temperature;
                    squared_difference_sum += difference * difference;
                }
            }

            const float variance = squared_difference_sum / (CONFIG::STACK_SIZE * CONFIG::TEMPERATURES_COUNT_PER_DEVICE);
            const float standardDeviation = sqrtf(variance);
            const float max_temp = data.cell_avg_temperature + standardDeviation;
            const float min_temp = data.cell_avg_temperature - standardDeviation;
            
            for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
            {
                for(size_t j = 0; j < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; j++)
                {
                    if(data.cell_temperatures[i][j] > max_temp or 
                       data.cell_temperatures[i][j] < min_temp)
                    {
                        data.cell_temperatures[i][j] = 0.f;
                    }
                }
            }
        }
        
        for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < CONFIG::CELL_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_voltages[i][j] > max_voltage) max_voltage = data.cell_voltages[i][j];
                if(data.cell_voltages[i][j] < min_voltage) min_voltage = data.cell_voltages[i][j];
                if(data.cell_voltages[i][j] > CONFIG::CELL_OV_FLOAT or data.cell_voltages[i][j] < CONFIG::CELL_UV_FLOAT) continue;
            }
            for(size_t j = 0; j < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_temperatures[i][j] > max_temperature) max_temperature = data.cell_temperatures[i][j];
                if(data.cell_temperatures[i][j] < min_temperature) min_temperature = data.cell_temperatures[i][j];
                if(data.cell_temperatures[i][j] > CONFIG::CELL_OT_FLOAT or data.cell_temperatures[i][j] < CONFIG::CELL_UT_FLOAT) continue;
            }
        }

        data.cell_max_temperature = max_temperature;
        data.cell_max_voltage = max_voltage;
        data.cell_min_temperature = min_temperature;
        data.cell_min_voltage = min_voltage;

        /* usb connected */
        data.vusb = HAL_ADC_GetValue(&hadc1);
        data.usb_connected = data.vusb > CONFIG::USB_VBUS_THRESH;

        /* AIR state machine */
        air_state_machine.update();

        /* Error checker */
        if(error_checker.check_errors(tx_time_get()))
        {
            data.error = true;
        }

        /* handle warning */
        if(data.warning) led_wrn.toggle();
        if(data.error) 
        {
            led_err.toggle();
            sig_err.set();
        }

        if(data.commands.save_config)
        {
            data.commands.save_config = false;
            HAL_FLASH_Unlock();
            {
                bool error { false };
                std::array<uint32_t, 8> ti { };

                for(size_t device = 0; device < CONFIG::STACK_SIZE; device++)
                {
                    for(size_t temp = 0; temp < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; temp++)
                    {
                        size_t index = device % 4;
                        size_t offset = (device / 4) * 8 + temp;
                        ti[index] |= (static_cast<uint32_t>(CONFIG::IGNORE_TEMPERATURES_MATRIX[device][temp]) << offset);
                    }
                }
                for(size_t i = 0; i < temp_ign.size(); i++)
                {
                    if(auto ret = temp_ign[i]->write(ti[i]); ret) error = true;
                }
                if(auto ret = i_calib_gain.write(CONFIG::CURRENT_GAIN); ret)        error = true;
                if(auto ret = i_calib_bias.write(CONFIG::CURRENT_BIAS); ret)        error = true;
                if(auto ret = acu_calib_gain.write(CONFIG::ACU_VOLTAGE_GAIN); ret)  error = true;
                if(auto ret = acu_calib_bias.write(CONFIG::ACU_VOLTAGE_BIAS); ret)  error = true;
                if(auto ret = car_calib_gain.write(CONFIG::CAR_VOLTAGE_GAIN); ret)  error = true;
                if(auto ret = car_calib_bias.write(CONFIG::CAR_VOLTAGE_BIAS); ret)  error = true;

                if(error) data.loggers.errors.log_error("SAVE CFG");
            }
            HAL_FLASH_Lock();
        }

        data.update_times.main = updates.update(tx_time_get());

        tx_thread_sleep(30);
    }
}

// TODO:
/**
 *  @brief  SOC update timer callback, it is used to update SOC every 50 ms
 *  @note   Definition in threads.hpp, time constant wasn't optimized properly 
 */
void soc_update_timer_callback(__unused ULONG arg)
{
    static UpdatesCounter updates;

    soc.update(data.acu_voltage, data.current, data.cell_avg_temperature);
    data.soc = soc.get();
    data.kalman.innovation = soc.get_innovation();
    data.kalman.k_soc = soc.get_k_soc();
    data.kalman.v1 = soc.get_v1();
    data.kalman.v2 = soc.get_v2();
    data.kalman.vh = soc.get_vh();
    data.kalman.dr = soc.get_dr();
    data.kalman.v_model = soc.get_v_model();


    data.update_times.soc = updates.update(tx_time_get());
}
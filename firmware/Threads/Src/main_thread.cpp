#include "main.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"
#include "array"

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
Gpio det_tsms(DET_TSMS_GPIO_Port, DET_TSMS_Pin, false);
Gpio det_charger(DET_CHARGER_GPIO_Port, DET_CHARGER_Pin, false);
// Gpio adc_dry(ADC_NDRY_GPIO_Port, ADC_NDRY_Pin, true);
// Gpio bq_flt(NFLT_GPIO_Port, NFLT_Pin, true);

constexpr Polynomial t_r_poly { Config::POLYNOMIAL_T_R };
constexpr auto r_u_lambda = [](float voltage) -> float
{
    return Config::NOMINAL_NTC_RESISTANCE / (Config::NOMINAL_TSREF - voltage) * voltage;
};

ChargerCanRxController charger_rx;

extern StateMachine air_state_machine;
extern StateMachine charger_state_machine;  

extern ErrorChecker error_checker;

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

    while(not (data.ads_init_done and data.bq_init_done))
    {
        tx_thread_sleep(10);
    }

    // init socs
    for(size_t i = 0; i < Config::STACK_SIZE; i++)  
    {
        for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
        {
            data.cell_socs[i][j].set_from_voltage(data.cell_voltages[i][j]);
        }
    }

    data.system_init_done = true;

    while(true)
    {
        /* Is alive */
        led_ok.toggle();

        /* Read gpios */
        data.tsms = det_tsms.read();
        data.on_charger = det_charger.read();

        /* Update SoC */
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
            {
                data.cell_socs[i][j].update(data.cell_voltages[i][j], data.current, data.on_charger);
            }
        }
        float soc_avg = 0.0f;
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
            {
                soc_avg += data.cell_socs[i][j].get();
            }
        }
        soc_avg /= Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE;
        data.soc = soc_avg;

        /* update true temps */
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                auto r = r_u_lambda(data.gpio_voltages[i][j]);
                data.cell_temperatures[i][j] = t_r_poly.evaluate(r) - 273.f;
            }
        }

        /* update min max */
        float max_voltage = 0.0f;
        float min_voltage = 100.f;
        float max_temperature = 0.0f;
        float min_temperature = 100.0f;
        float accumulator_voltage = 0.0f;
        float accumulator_temperature = 0.0f;
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_voltages[i][j] > max_voltage) max_voltage = data.cell_voltages[i][j];
                if(data.cell_voltages[i][j] < min_voltage) min_voltage = data.cell_voltages[i][j];
                if(data.cell_voltages[i][j] > Config::CELL_OV_FLOAT or data.cell_voltages[i][j] < Config::CELL_UV_FLOAT) continue;
                accumulator_voltage += data.cell_voltages[i][j];
            }
            for(size_t j = 0; j < Config::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_temperatures[i][j] > max_temperature) max_temperature = data.cell_temperatures[i][j];
                if(data.cell_temperatures[i][j] < min_temperature) min_temperature = data.cell_temperatures[i][j];
                if(data.cell_temperatures[i][j] > Config::CELL_OT_FLOAT or data.cell_temperatures[i][j] < Config::CELL_UT_FLOAT) continue;
                accumulator_temperature += data.cell_temperatures[i][j];
            }
        }
        data.cell_max_temperature = max_temperature;
        data.cell_max_voltage = max_voltage;
        data.cell_min_temperature = min_temperature;
        data.cell_min_voltage = min_voltage;
        data.cell_avg_voltage = accumulator_voltage / (Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE);
        data.cell_avg_temperature = accumulator_temperature / (Config::STACK_SIZE * Config::TEMPERATURES_COUNT_PER_DEVICE);

        /* AIR state machine */
        air_state_machine.update();

        /* Error checker */
        if constexpr (Config::TURN_OFF_ERRORS)
        {
            data.error = false;
        }
        else /* constexpr */
        {
            if(error_checker.check_errors(tx_time_get()))
            {
                data.error = true;
            }
        }

        /* handle warning */
        if(data.warning) led_wrn.toggle();
        if(data.error) led_err.toggle();

        data.update_times.main_updates_per_sec = updates.update(tx_time_get());

        tx_thread_sleep(50);
    }
}

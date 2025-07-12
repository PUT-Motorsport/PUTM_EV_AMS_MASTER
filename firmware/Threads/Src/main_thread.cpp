#include "main.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"
#include "array"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "wrapper/fdcan.hpp"
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

using namespace PUTM;
using namespace Utils;

/* Wrappers */

Gpio led_err(LED_ERROR_GPIO_Port, LED_ERROR_Pin, true);
Gpio led_wrn(LED_WARNING_GPIO_Port, LED_WARNING_Pin, true);
Gpio led_ok(LED_OK_GPIO_Port, LED_OK_Pin, true);
Gpio sig_err(SIG_AMS_ERROR_GPIO_Port, SIG_AMS_ERROR_Pin, true);
// Gpio en_12v(EN_12V_GPIO_Port, EN_12V_Pin, false);
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

std::array<SoC, Config::STACK_SIZE * Config::CELL_COUNT_PER_DEVICE> socs;

ChargerCanRxController charger_rx;

extern StateMachine air_state_machine;
extern StateMachine charger_state_machine;  

extern ErrorChecker error_checker;

VOID main_thread_entry(__unused ULONG thread_input)
{
    led_wrn.reset();
    led_ok.reset();
    led_err.reset();
    sig_err.reset();
    sig_air_pre.reset();
    sig_air_p.reset();
    sig_air_m.reset();

    // init_air_state_machine(&air_state_machine);
    // init_error_checker(&error_checker);

    tx_thread_sleep(100);

    // init socs
    for(size_t i = 0; i < Config::STACK_SIZE; i++)
    {
        for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
        {
            // socs[i * Config::CELL_COUNT_PER_DEVICE + j].set_from_voltage(data.cell_voltages[i * Config::STACK_SIZE + j]);
        }
    }

    while(true)
    {
#ifdef DEBUG_PRINTF_ENABLE
        // FIXME: this is a temporary solution, change it to real one
        char buffer { 0 };
        HAL_UART_Receive(&huart1, (uint8_t*)&buffer, 1, 10);
        if(buffer == '1')
        {
            data.cmd_hv = true;
        }
        if(buffer == '0')
        {
            data.cmd_charger = true;
        }

        char buffer2[128] { 0 };
        /* Print clear terminal command */
        // snprintf(buffer2, sizeof(buffer2), "%c%c%c%c",0x1B,0x5B,0x32,0x4A);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        /* Print cmd_hv */
        // snprintf(buffer2, sizeof(buffer2), "Info: cmd_hv: %d\n", data.cmd_hv);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print cmd_charger */
        // snprintf(buffer2, sizeof(buffer2), "Info: cmd_charger: %d\n", data.cmd_charger);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print current air state machine name */
        // snprintf(buffer2, sizeof(buffer2), "Info: air: %s\n", air_state_machine.current_state->name);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print charger current state machine name */
        // snprintf(buffer2, sizeof(buffer2), "Info: charger: %s\n", charger_state_machine.current_state->name);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // // /* Print tsms state */
        // snprintf(buffer2, sizeof(buffer2), "Info: tsms: %d\n", data.tsms);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print acu voltage */
        // snprintf(buffer2, sizeof(buffer2), "Info: acu: %.2f\n", data.acu_voltage);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print car voltage */
        // snprintf(buffer2, sizeof(buffer2), "Info: car: %.2f\n", data.car_voltage);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print current */
        // snprintf(buffer2, sizeof(buffer2), "Info: current: %.2f\n", data.current);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
        // /* Print erro state from data */
        // snprintf(buffer2, sizeof(buffer2), "Info: error: %d\n", data.error);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen(buffer2), 100);
#endif /* DEBUG_PRINTF_ENABLE */
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
                // socs[i * Config::CELL_COUNT_PER_DEVICE + j].update(data.cell_voltages[i * Config::STACK_SIZE + j], data.current, data.on_charger);
            }
        }

        /* AIR state machine */
        air_state_machine.update();

        /* Error checker */
        if(error_checker.check_errors(tx_time_get()))
        {
            Error *error = error_checker.get_next_error();
            while(error != nullptr)
            {
                /* Print error */
                char buffer[128] { 0 };
                snprintf(buffer, sizeof(buffer), "Error: %s\n", error->parse(error->last_code));
                HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);

                error = error_checker.get_next_error();
            }
            data.error = true;
        }
        tx_thread_sleep(50);
    }
}

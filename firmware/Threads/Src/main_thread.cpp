#include "main.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "main/air_state_machine.hpp"
#include "error_checker.hpp"
#include "main/error_checker.hpp"

using namespace PUTM;

/* Wrappers */

Gpio led_err(LED_ERROR_GPIO_Port, LED_ERROR_Pin, true);
Gpio led_wrn(LED_WARNING_GPIO_Port, LED_WARNING_Pin, true);
Gpio led_ok(LED_OK_GPIO_Port, LED_OK_Pin, true);
Gpio sig_err(SIG_AMS_ERROR_GPIO_Port, SIG_AMS_ERROR_Pin, true);
Gpio en_12v(EN_12V_GPIO_Port, EN_12V_Pin, false);
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

extern StateMachine air_state_machine;  

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
    en_12v.reset();

    init_air_state_machine(&air_state_machine);
    init_error_checker(&error_checker);

    while(true)
    {
        /* Is alive */
        led_ok.toggle();

        /* Pool trivial data */
        data.tsms = det_tsms.read();
        data.on_charger = det_charger.read();

        /* AIR state machine */
        air_state_machine.update();

        /* Error checks */
        if(error_checker.check_errors(tx_time_get()))
        {
            Error *error = error_checker.get_next_error();
            while(error != nullptr)
            {
                error = error_checker.get_next_error();
            }
            data.error = true;
        }
        tx_thread_sleep(50);
    }
}

#include "main.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "main_thread/state_machin_def.hpp"

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

VOID main_thread_entry(__unused ULONG thread_input)
{
    state_machine.add_edges(idle_to_precharge, idle_to_error, precharge_to_hv_on, 
                            precharge_to_error, hv_on_to_idle, hv_on_to_error);
    state_machine.start(&idle);

    while(true)
    {
        led_ok.toggle();
        state_machine.update();
        tx_thread_sleep(50);
    }
}

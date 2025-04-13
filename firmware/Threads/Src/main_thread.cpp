#include "main.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"

using namespace PUTM;

/* Wrappers */

static Gpio led_err(LED_ERROR_GPIO_Port, LED_ERROR_Pin, true);
static Gpio led_wrn(LED_WARNING_GPIO_Port, LED_WARNING_Pin, true);
static Gpio led_ok(LED_OK_GPIO_Port, LED_OK_Pin, true);
static Gpio sig_err(SIG_AMS_ERROR_GPIO_Port, SIG_AMS_ERROR_Pin, true);
static Gpio en_12v(EN_12V_GPIO_Port, EN_12V_Pin, false);
static Gpio sig_air_pre(SIG_AIR_PRE_GPIO_Port, SIG_AIR_PRE_Pin, false);
static Gpio sig_air_p(SIG_AIR_P_GPIO_Port, SIG_AIR_P_Pin, false);
static Gpio sig_air_m(SIG_AIR_M_GPIO_Port, SIG_AIR_M_Pin, false);
static Gpio det_air_pre(DET_AIR_PRE_GPIO_Port, DET_AIR_PRE_Pin, false);
static Gpio det_air_p(DET_AIR_P_GPIO_Port, DET_AIR_P_Pin, false);
static Gpio det_air_m(DET_AIR_M_GPIO_Port, DET_AIR_M_Pin, false);
static Gpio det_tsms(DET_TSMS_GPIO_Port, DET_TSMS_Pin, false);
static Gpio det_charger(DET_CHARGER_GPIO_Port, DET_CHARGER_Pin, false);
// Gpio adc_dry(ADC_NDRY_GPIO_Port, ADC_NDRY_Pin, true);
// Gpio bq_flt(NFLT_GPIO_Port, NFLT_Pin, true);

/**
 *  @brief  turn off hv
 */
void hv_off()
{
    sig_air_p.reset();
    sig_air_m.reset();
    sig_air_pre.reset();
}

/**
 *  @brief  start hv precharge
 */
void hv_precharge()
{
    sig_air_p.reset();
    sig_air_m.set();
    sig_air_pre.set();
}

/**
 *  @brief  turn on hv
 */
void hv_on()
{
    sig_air_p.set();
    sig_air_m.set();
    sig_air_pre.reset();
}

/* State machine */

/**
 *  @brief  Idle state
 *  @note   In this state not much happens the state machine set GPIO pins to their desired 
 *          default state
 */
static State idle
{
    .name = "idle",
    .on_enter = []()
    { 
        led_err.reset(); 
        led_wrn.reset();

        sig_err.reset();
        hv_off();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};


static uint32_t precharge_on_enter_tick;
/**
 *  @brief  Start precharge
 *  @note   In this state AIR- and precharge relay are activated to start precharge process
 */
static State precharge
{
    .name = "precharge",
    .on_enter = []()
    {
        precharge_on_enter_tick = HAL_GetTick();
        hv_precharge();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  On state
 */
static State on
{
    .name = "on",
    .on_enter = []()
    { 
        hv_on();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Error state
 *  @note   If AMS detects an error the machine will be stuck in this state 
 */
static State error
{
    .name = "error",
    .on_enter = []()
    { 
       hv_off();
    },
    .on_update = []()
    { 
        // Flash error state
        led_err.toggle();
    },
    // .on_exit = [](){ }
};

static StateEdge idle_to_precharge
{
    .name = "idle -> precharge",
    .condition = []() -> bool
    { 
        return (data.car_voltage <= Config::HV_THRESH and not data.error and 
                data.tsms and data.cmd_on); 
    },
    .prev_state = &idle,
    .next_state = &precharge,
};

static StateEdge idle_to_error
{
    .name = "idle -> error",
    .condition = []() -> bool
    { 
        return (data.car_voltage > Config::HV_THRESH or data.error); 
    },
    .prev_state = &idle,
    .next_state = &error,
};

static StateEdge precharge_to_on
{
    .name = "precharge -> on",
    .condition = []() -> bool
    {
        float car_thresh = data.acu_voltage * Config::CAR_CHARGE_THRESH;
        uint32_t time = HAL_GetTick() - precharge_on_enter_tick;
        return (time > Config::PRECHARGE_MIN_WAIT and data.car_voltage >= car_thresh and not data.error); 
    },
    .prev_state = &precharge,
    .next_state = &on,
};

// static StateEdge precharge_to_idle
// {
//     .name = "precharge -> idle",
//     .condition = []() -> bool
//     {
//         return (not data.tsms or not data.cmd_on);
//     },
//     .prev_state = &precharge,
//     .next_state = &on,
// };

static StateEdge precharge_to_error
{
    .name = "precharge -> error",
    .condition = []() -> bool
    { 
        float car_thresh = data.acu_voltage * Config::CAR_CHARGE_THRESH;
        uint32_t time = HAL_GetTick() - precharge_on_enter_tick;
        return (((data.car_voltage <= Config::HV_THRESH and time > Config::PRECHARGE_MIN_WAIT) or
                (data.car_voltage < car_thresh and time > Config::PRECHARGE_MAX_WAIT) or
                (data.error)) and not
                (data.tsms or data.cmd_on));
    },
    .prev_state = &precharge,
    .next_state = &error,
};

static StateEdge on_to_idle
{
    .name = "on -> idle",
    .condition = []() -> bool
    {
        return ((not data.cmd_on or not data.tsms) and not data.error); 
    },
    .prev_state = &on,
    .next_state = &idle,
};

static StateEdge on_to_error
{
    .name = "on -> error",
    .condition = []() -> bool
    { 
        return ((data.car_voltage <= Config::HV_THRESH or data.error) and not
                (data.cmd_on or data.tsms)); 
    },
    .prev_state = &on,
    .next_state = &error,
};

static StateMachine state_machine;

/* Error handler */

static Error com_error
{
    .name = "Communication error",
    .timeout
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

static Error vcell_error
{
    .name = "Cell voltage error",
    .condition = []() -> uint32_t 
    {

    }
};

static Error tcell_error
{
    .name = "Temperatures voltage error",
    .condition = []() -> uint32_t 
    {

    }
};

static Error current_error
{
    .name = "Current error",
    .condition = []() -> uint32_t 
    {

    }
};

static Error v_error
{
    .name = "Voltage Car/Acu error",
    .condition []() -> uint32_t 
    {

    }
};

VOID main_thread_entry(__unused ULONG thread_input)
{
    // TODO: add from pre to idle when tsms signal disappears?
    state_machine.add_edges(idle_to_precharge, idle_to_error, precharge_to_on, 
                            precharge_to_error, on_to_idle, on_to_error);
    state_machine.start(&idle);

    while(true)
    {
        led_ok.toggle();
        state_machine.update();
        tx_thread_sleep(50);
    }
}

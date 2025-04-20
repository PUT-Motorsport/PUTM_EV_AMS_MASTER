#include "main.h"
#include "tx_api.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"

using namespace PUTM;

extern Gpio sig_air_p;
extern Gpio sig_air_m;
extern Gpio sig_air_pre;
extern Gpio led_ok;
extern Gpio led_err;
extern Gpio led_wrn;
extern Gpio sig_err;

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

/* HV on state machine */

/**
 *  @brief  Idle state
 *  @note   In this state not much happens the state machine set GPIO pins to their desired 
 *          default state
 */
State idle
{
    .name = "idle",
    .on_enter = []()
    { 
        hv_off();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

uint32_t precharge_on_enter_tick;
/**
 *  @brief  Start precharge
 *  @note   In this state AIR- and precharge relay are activated to start precharge process
 */
State precharge
{
    .name = "precharge",
    .on_enter = []()
    {
        precharge_on_enter_tick = tx_time_get();
        hv_precharge();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  On state
 */
State on
{
    .name = "on",
    .on_enter = []()
    { 
        sig_err.reset();
        led_wrn.reset();
        led_ok.set();
        led_err.reset();
        hv_on();
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Error state
 *  @note   If AMS detects an error the machine will be stuck in this state 
 */
State error
{
    .name = "error",
    .on_enter = []()
    { 
       hv_off();
       sig_err.set();
    },
    .on_update = []()
    { 
        // Flash error state
        led_err.toggle();
        // Error_Handler();
        return;
    },
    // .on_exit = [](){ }
};

StateEdge idle_to_precharge
{
    .name = "idle -> precharge",
    .condition = []() -> bool
    { 
        return (data.car_voltage <= Config::MIN_HV_THRESH and not data.error and 
                data.tsms and data.cmd_on); 
    },
    .prev_state = &idle,
    .next_state = &precharge,
};

StateEdge idle_to_error
{
    .name = "idle -> error",
    .condition = []() -> bool
    { 
        return (data.car_voltage > Config::MIN_HV_THRESH or data.error); 
    },
    .prev_state = &idle,
    .next_state = &error,
};

StateEdge precharge_to_on
{
    .name = "precharge -> on",
    .condition = []() -> bool
    {
        float car_thresh = data.acu_voltage * Config::CAR_CHARGE_THRESH;
        uint32_t time = HAL_GetTick() - precharge_on_enter_tick;
        return (time > Config::MIN_PRECHARGE_WAIT and data.car_voltage >= car_thresh and not data.error); 
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

StateEdge precharge_to_error
{
    .name = "precharge -> error",
    .condition = []() -> bool
    { 
        float car_thresh = data.acu_voltage * Config::CAR_CHARGE_THRESH;
        uint32_t time = HAL_GetTick() - precharge_on_enter_tick;
        return (((data.car_voltage <= Config::MIN_HV_THRESH and time > Config::MIN_PRECHARGE_WAIT) or
                (data.car_voltage < car_thresh and time > Config::MAX_PRECHARGE_WAIT) or
                (data.error)) and not
                (data.tsms or data.cmd_on));
    },
    .prev_state = &precharge,
    .next_state = &error,
};

StateEdge on_to_idle
{
    .name = "on -> idle",
    .condition = []() -> bool
    {
        return ((not data.cmd_on or not data.tsms) and not data.error); 
    },
    .prev_state = &on,
    .next_state = &idle,
};

//FIXME: add discharge state
// StateEdge on_to_discharge
// {
//     .name = "on -> discharge",
//     .condition = []() -> bool
//     {
//         // TODO: add discharge condition
//     },
//     .prev_state = &on,
//     .next_state = &discharge,
// };

StateEdge on_to_error
{
    .name = "on -> error",
    .condition = []() -> bool
    { 
        return ((data.car_voltage <= Config::MIN_HV_THRESH or data.error) and not
                (data.cmd_on or data.tsms)); 
    },
    .prev_state = &on,
    .next_state = &error,
};

#ifdef TEST_MODE_1  
StateEdge reset
{
    .name = "error -> idle",
    .condition = []() -> bool
    { 
        return (data.reset_state_machine); 
    },
    .prev_state = &error,
    .next_state = &idle,
};
#endif /* TEST_MODE_1 */

StateMachine air_state_machine;

void init_air_state_machine(StateMachine *sm)
{
    sm->add_edges(
        idle_to_precharge, 
        idle_to_error, 
        precharge_to_on, 
        precharge_to_error, 
        on_to_idle, 
        on_to_error);
#ifdef TEST_MODE_1
    sm->add_edges(reset);
#endif /* TEST_MODE_1 */
    sm->start(&idle);
}
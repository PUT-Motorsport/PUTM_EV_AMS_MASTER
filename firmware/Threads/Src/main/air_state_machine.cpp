#include "main.h"
#include "tx_api.h"
#include "usart.h"
#include "cstdio"

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

/* When state machine has entered the off state*/
/**
 *  @brief  Off state
 *  @note   In this state the system is off, no relays are activated and
 *          no commands are executed, the system is waiting for charger to be plugged if
 *          the charger is not plugged into charge for some time the system 
 *          will enter idle state
 */
uint32_t off_enter_tick;
State off
{
    .name = "off",
    .on_enter = []() 
    {
        hv_off();
        off_enter_tick = tx_time_get();
        data.cmd_hv = false;
        data.precharge = false;
        data.hv_on = false;
    },
    .on_update = []()
    {
        data.cmd_hv = false;
        data.precharge = false;
        data.hv_on = false;
    },
    // .on_exit = []() { },
};

/* When state machine has entered the idle state */
uint32_t idle_enter_tick;
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
        idle_enter_tick = tx_time_get();
        hv_off();
    },
    .on_update = []()
    {
        if(not data.tsms)
        {
            data.cmd_hv = false;
        }
    },
    .on_exit = []()
    {
        data.cmd_hv = false;
    },
};

/* When state machine has entered the precharge state */
uint32_t precharge_enter_tick;
/**
 *  @brief  Start precharge
 *  @note   In this state AIR- and precharge relay are activated to start precharge process
 */
State precharge
{
    .name = "precharge",
    .on_enter = []()
    {
        precharge_enter_tick = tx_time_get();
        data.precharge = true;
        hv_precharge();
    },
    // .on_update = [](){ },
    .on_exit = []()
    {
        data.precharge = false;
        data.cmd_hv = false;
    },
};

/**
 *  @brief  On state
 */
State on
{
    .name = "on",
    .on_enter = []()
    { 
        data.hv_on = true;
        hv_on();
    },
    // .on_update = []()
    .on_exit = []()
    {
        data.hv_on = false;
        data.cmd_hv = false;
    },
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
        // led_err.toggle();
        data.precharge = false;
        data.cmd_hv = false;
        // Error_Handler();
        return;
    },
    // .on_exit = [](){ }
};

// StateEdge any_to_error
// {
//     .name = "any -> error",
//     .condition = []() -> bool
//     {
//         return data.error;
//     },
//     .prev_state = &StateMachine::any_state,
//     .next_state = &error
// };

StateEdge off_to_idle
{
    .name = "off -> idle",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - off_enter_tick;
        return (not data.on_charger and time > Config::STATE_MACHINE_OFF_TO_IDLE_WAIT);
    },
    .prev_state = &off,
    .next_state = &idle,
};

StateEdge idle_to_off
{
    .name = "idle -> off",
    .condition = []() -> bool
    {
        return (data.on_charger);
    },
    .prev_state = &idle,
    .next_state = &off,
};

StateEdge idle_to_precharge
{ 
    .name = "idle -> precharge",
    .condition = []() -> bool
    { 
        uint32_t time = tx_time_get() - idle_enter_tick;
        // use the error?
        return (data.car_voltage <= Config::MIN_HV_THRESH and 
                data.tsms and data.cmd_hv and time > Config::STATE_MACHINE_IDLE_TO_PRECHARGE_WAIT); 
    },
    .prev_state = &idle,
    .next_state = &precharge,
};

StateEdge idle_to_error
{
    .name = "idle -> error",
    .condition = []() -> bool
    { 
        return (data.error); //data.car_voltage > Config::MIN_HV_THRESH or 
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
        uint32_t time = tx_time_get() - precharge_enter_tick;
        return time > Config::MIN_PRECHARGE_WAIT and data.car_voltage >= car_thresh; 
    },
    .prev_state = &precharge,
    .next_state = &on,
};

static StateEdge precharge_to_idle
{
    .name = "precharge -> idle",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - precharge_enter_tick;
        return ((data.cmd_hv and time > Config::STATE_MACHINE_IDLE_TO_PRECHARGE_WAIT) or not data.tsms);// or not data.cmd_hv
    },
    .prev_state = &precharge,
    .next_state = &idle,
};

StateEdge precharge_to_error
{
    .name = "precharge -> error",
    .condition = []() -> bool
    { 
        // float car_thresh = data.acu_voltage * Config::CAR_CHARGE_THRESH;
        uint32_t time = tx_time_get() - precharge_enter_tick;

        bool precharge_timeout_lo = (data.car_voltage <= Config::MIN_HV_THRESH and time > Config::MIN_PRECHARGE_WAIT);
        bool precharge_timeout_hi = (time > Config::MAX_PRECHARGE_WAIT);

        bool error = precharge_timeout_lo or precharge_timeout_hi or data.error; //  and not (data.tsms or data.cmd_hv))

        return error;
    },
    .prev_state = &precharge,
    .next_state = &error,
};

StateEdge on_to_idle
{
    .name = "on -> idle",
    .condition = []() -> bool
    {
        return (data.tsms);
    },
    .prev_state = &on,
    .next_state = &idle,
};

//TODO: add discharge state
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
        return (data.error);
    },
    .prev_state = &on,
    .next_state = &error,
};

StateMachine air_state_machine;

void init_air_state_machine(StateMachine *sm)
{
    sm->add_edges(
        off_to_idle,
        idle_to_off,
        idle_to_precharge, 
        idle_to_error, 
        precharge_to_on, 
        precharge_to_idle,
        precharge_to_error, 
        on_to_idle, 
        on_to_error);
#ifdef TEST_MODE_1
    sm->add_edges(reset);
#endif /* TEST_MODE_1 */
    sm->start(&off);
}
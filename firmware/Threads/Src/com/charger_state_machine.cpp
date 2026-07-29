#include "main.h"
#include "tx_api.h"
#include "usart.h"
#include "cstdio"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "com/charger_state_machine.hpp"
#include "charger.hpp"
#include "wrapper/fdcan.hpp"    
#include <cstdint>

using namespace PUTM;

extern Gpio sig_air_p;
extern Gpio sig_air_m;
extern Gpio sig_air_pre;
extern Gpio led_ok;
extern Gpio led_err;
extern Gpio led_wrn;
extern Gpio sig_err;

uint32_t charger_off_enter_tick;

State charger_off
{
    .name = "off",
    .on_enter = []() 
    {  
        sig_air_p.reset();
        sig_air_m.reset();
        sig_air_pre.reset();
        charger_off_enter_tick = tx_time_get();
    },
    .on_update = []() 
    { 
        data.cmd_charger = false;
    },
    // .on_exit = []() { },
};

static ChargerCanRxController charger_rx { };

uint32_t charger_idle_enter_tick;

State charger_idle
{
    .name = "idle",
    .on_enter = []() 
    { 
        sig_air_p.reset();
        sig_air_m.reset();
        sig_air_pre.reset();
    },
    .on_update = []() 
    { 
        charger_rx.update();
        ChargerCanTxMessage frame
		{
			0,
			0,
			false
		};
        auto status = frame.send();
    },
    .on_exit = []() 
    { 
        // data.cmd_charger = false;
    },
};

uint32_t charger_precharge_enter_tick;

State charger_precharge
{
    .name = "precharge",
    .on_enter = []() 
    { 
        sig_air_p.reset();
        sig_air_m.set();
        sig_air_pre.reset();
        charger_precharge_enter_tick = tx_time_get();
    },
    .on_update = []() 
    { 
        charger_rx.update();
        ChargerCanTxMessage frame
		{
			0,
			0,
			false
		};
        auto status = frame.send();
    },
    .on_exit = []() 
    { 
        data.cmd_charger = false;
    },
};

State charger_on
{
    .name = "on",
    .on_enter = []() 
    { 
        sig_air_p.set();
        sig_air_m.set();
        sig_air_pre.reset();
    },
    .on_update = []() 
    { 
        charger_rx.update();
        ChargerCanTxMessage frame
		{
			Config::MAX_CHARGING_VOLTAGE,
            data.charging_current,
            data.cmd_charger
		};
        auto status = frame.send();
    },
    .on_exit = []() 
    { 
        data.cmd_charger = false;
    },
};

State charger_error
{
    .name = "error",
    .on_enter = []()
    { 
        sig_air_p.reset();
        sig_air_m.reset();
        sig_air_pre.reset();
        data.error = true;
    },
    .on_update = []()
    { 
        charger_rx.update();
        ChargerCanTxMessage frame
		{
			0.f,
			0.f,
            false
		};
        auto status = frame.send();
        // Flash error state
        // led_err.toggle();
        data.precharge = false;
        data.cmd_hv = false;
        // Error_Handler();
        return;
    },
    // .on_exit = [](){ }
};

StateEdge charrger_any_to_error
{
    .name = "any -> error",
    .condition = []() -> bool
    {
        return data.error;
    },
    .prev_state = &StateMachine::any_state,
    .next_state = &charger_error
};

StateEdge charger_off_to_idle
{
    .name = "off -> idle",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - charger_off_enter_tick;
        return (data.on_charger and time > Config::STATE_MACHINE_OFF_TO_IDLE_WAIT);
    },
    .prev_state = &charger_off,
    .next_state = &charger_idle,
};

StateEdge charger_idle_to_off
{
    .name = "idle -> off",
    .condition = []() -> bool
    {
        return not data.on_charger;
    },
    .prev_state = &charger_idle,
    .next_state = &charger_off,
};

StateEdge charger_idle_to_precharge
{
    .name = "idle -> precharge",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - charger_idle_enter_tick;
        return (data.on_charger and data.tsms and time > Config::STATE_MACHINE_IDLE_TO_PRECHARGE_WAIT);
    },
    .prev_state = &charger_idle,
    .next_state = &charger_precharge,
};

StateEdge charger_precharge_to_on
{
    .name = "precharge -> on",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - charger_precharge_enter_tick;
        return (time > Config::MIN_PRECHARGE_WAIT); // and data.car_voltage >= data.acu_voltage * Config::CAR_CHARGE_THRESH);
    },
    .prev_state = &charger_precharge,
    .next_state = &charger_on,
};

StateEdge charger_precharge_to_idle
{
    .name = "precharge -> idle",
    .condition = []() -> bool
    {
        uint32_t time = tx_time_get() - charger_precharge_enter_tick;
        return (not data.on_charger or not data.tsms or time > Config::MAX_PRECHARGE_WAIT);
    },
    .prev_state = &charger_precharge,
    .next_state = &charger_idle,
};

StateEdge charger_on_to_idle
{
    .name = "on -> idle",
    .condition = []() -> bool
    {
        return not data.on_charger or not data.tsms;
    },
    .prev_state = &charger_on,
    .next_state = &charger_idle,
};

StateMachine charger_state_machine;

void init_charger_state_machine(StateMachine *sm)
{
    sm->add_edges(
        charrger_any_to_error,
        charger_off_to_idle,
        charger_idle_to_off,
        charger_idle_to_precharge,
        charger_precharge_to_on,
        charger_precharge_to_idle,
        charger_on_to_idle);
    sm->start(&charger_off);
}
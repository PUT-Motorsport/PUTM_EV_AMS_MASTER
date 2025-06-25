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

using namespace PUTM;

State charger_off
{
    .name = "off",
    // .on_enter = []() {  },
    .on_update = []() 
    { 
        data.cmd_charger = false;
    },
    // .on_exit = []() { },
};

static ChargerCanRxController charger_rx { };

State charger_idle
{
    .name = "idle",
    // .on_enter = []() {  },
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

    },
    .on_update = []() 
    { 
        charger_rx.update();
        ChargerCanTxMessage frame
		{
			Config::CHARGING_VOLTAGE,
            data.charging_current,
            true
		};
        auto status = frame.send();
    },
    .on_exit = []() 
    { 
        data.cmd_charger = false;
    },
};

StateEdge charger_off_to_idle
{
    .name = "off -> idle",
    .condition = []() -> bool
    {
        return data.on_charger;
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

StateEdge charger_idle_to_on
{
    .name = "idle -> on",
    .condition = []() -> bool
    {
        return data.hv_on and data.cmd_charger;
    },
    .prev_state = &charger_idle,
    .next_state = &charger_on,
};

StateEdge charger_on_to_idle
{
    .name = "on -> idle",
    .condition = []() -> bool
    {
        return not data.hv_on or data.cmd_charger;
    },
    .prev_state = &charger_on,
    .next_state = &charger_idle,
};

StateMachine charger_state_machine;

void init_charger_state_machine(StateMachine *sm)
{
    sm->add_edges(
        charger_off_to_idle,
        charger_idle_to_off,
        charger_idle_to_on,
        charger_on_to_idle);
    sm->start(&charger_off);
}
#include "main.h"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"

using namespace PUTM;

State car_can
{
    .name = "car can",
    .on_enter = []()
    { 
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

State charger_can
{
    .name = "charger can",
    .on_enter = []()
    { 
    },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

StateEdge car_to_charger
{
    .name = "car -> charger",
    .condition = []() -> bool
    { 
        return (data.on_charger and not data.error); 
    },
    .prev_state = &car_can,
    .next_state = &charger_can,
};
StateEdge charger_to_car
{
    .name = "charger -> car",
    .condition = []() -> bool
    { 
        return (not data.on_charger and not data.error); 
    },
    .prev_state = &charger_can,
    .next_state = &car_can,
};

StateMachine can_state_machine;

void init_can_state_machine(StateMachine *sm)
{
    sm->add_edges(
        car_to_charger,
        charger_to_car
    );
    sm->start(&car_can);
}
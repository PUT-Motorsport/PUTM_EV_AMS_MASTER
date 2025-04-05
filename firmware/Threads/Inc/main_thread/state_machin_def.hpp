#pragma once 

#include "state_machine.hpp"

/* state machine */

static PUTM::State idle
{
    .name = "idle",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

static PUTM::State precharge
{
    .name = "precharge",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

static PUTM::State hv_on
{
    .name = "hv_on",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

static PUTM::State error
{
    .name = "error",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

static PUTM::StateEdge idle_to_precharge
{
    .name = "idle -> precharge",
    .condition = [](){ return false; },
    .prev_state = &idle,
    .next_state = &precharge,
};

static PUTM::StateEdge idle_to_error
{
    .name = "idle -> error",
    .condition = [](){ return false; },
    .prev_state = &idle,
    .next_state = &error,
};

static PUTM::StateEdge precharge_to_hv_on
{
    .name = "precharge -> hv_on",
    .condition = [](){ return false; },
    .prev_state = &precharge,
    .next_state = &hv_on,
};

static PUTM::StateEdge precharge_to_error
{
    .name = "precharge -> error",
    .condition = [](){ return false; },
    .prev_state = &precharge,
    .next_state = &error,
};

static PUTM::StateEdge hv_on_to_idle
{
    .name = "hv_on -> idle",
    .condition = [](){ return false; },
    .prev_state = &hv_on,
    .next_state = &idle,
};

static PUTM::StateEdge hv_on_to_error
{
    .name = "hv_on -> error",
    .condition = [](){ return false; },
    .prev_state = &hv_on,
    .next_state = &error,
};

static PUTM::StateMachine state_machine;
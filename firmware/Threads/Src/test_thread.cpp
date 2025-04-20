/**
 *  @file   test_thread.cpp
 *  @brief  Test thread for the system. This thread is used to test the system and to check if the system is working properly.
 *  @note   This thread is used only for testing purposes, it is not used in the final application. Define the TEST_MODE_1 macro
 *          to enable this thread.
 */

#ifdef TEST_MODE_1
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "math"

using namespace PUTM;

/**
 *  @brief  Test init state
 */
State test_init_state
{
    .name = "test init state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Test idle state
 */
State test_idle_state 
{
    .name = "test idle state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Test precharge state
 */
State test_precharge_state 
{
    .name = "test precharge state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Test on state 
 *  @note   This state is supposed to mimic the the on state of the car
 */
State test_on_state
{
    .name = "test on state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

/**
 *  @brief  Test discharge state
 *  @note   This state is supposed to mimic the discharging of hv caps
 */
State test_discharge_state
{
    .name = "test discharge state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

VOID test_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        tx_thread_sleep(100);

    }
}
#endif /* TEST_MODE_1 */
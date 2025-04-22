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
/**
 *  @note This thread is used only for testing purposes, it is not used in the final application
 *        it is used to test the system and to check if the system is working properly. Define the
 *        TEST_MODE_1 macro to enable this thread.
 */

using namespace PUTM;

State test_init_state
{
    .name = "test init state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

State test_idle_state 
{
    .name = "test idle state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

State test_precharge_state 
{
    .name = "test precharge state",
    // .on_enter = [](){ },
    // .on_update = [](){ },
    // .on_exit = [](){ }
};

State test_hv_on_state 
{
    .name = "test hv on state",
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
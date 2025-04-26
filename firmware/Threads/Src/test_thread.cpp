/**
 *  @file   test_thread.cpp
 *  @brief  Test thread for the system. This thread is used to test the system and to check if the system is working properly.
 *          If DEBUG_PRINTF is defined the debug message will be transmited via uart
 *  @note   This thread is used only for testing purposes, it is not used in the final application. Define the TEST_MODE_1 macro
 *          to enable this thread.
 */

#ifdef DEBUG_TEST_MODE_1
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "math"

using namespace PUTM;

extern StateMachine air_state_machine;
extern State error;
extern State idle;
extern State precharge;
extern State on;
extern State discharge;

struct Test
{
    /* Test name */
    const char *name { "" };
    /* Test function */
    bool (*test)(void) { nullptr };
};

Test test_idle1
{
    .name = "test idle 1",
    .test = []()
    {
        /* Set data cell voltages and ovuv for all devices to default */
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT; j++)
            {
                data.cell_voltages[i][j] = 0.f;
                data.cell_ovuv[i][j] = false;
            }
        }
        /* Set data cell temperatures and otut for all devices to default */
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::TEMPERATURES_COUNT; j++)
            {
                data.cell_temperatures[i][j] = 0.f;
                data.cell_otut[i][j] = false;
            }
        }
        /* Set battery voltage to default */
        data.acu_voltage = 550.f;
        /* Set car voltage to default */
        data.car_voltage = 0.f;
        /* Set current to default */
        data.current = 0.f;
        /* Reset cmd */
        data.cmd_on = false;
        /* Reset error */
        data.error = false;
        /* Reset tsms */
        data.tsms = false;



        /* Force idle state */
        air_state_machine.set_current_state(&idle);
        /* Wait a bit */
        tx_thread_sleep(500);
        /* Check if the state machine is in idle state */
        if(air_state_machine.current_state == &idle) return true
        return false;
    },  
}

VOID test_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        tx_thread_sleep(100);
    }
}
#endif /* TEST_MODE_1 */
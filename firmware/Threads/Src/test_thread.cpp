/**
 *  @file   test_thread.cpp
 *  @brief  Test thread for the system. This thread is used to test the system and to check if the system is working properly.
 *          If DEBUG_PRINTF is defined the debug message will be transmited via uart
 *  @note   This thread is used only for testing purposes, it is not used in the final application. Define the TEST_MODE_1 macro
 *          to enable this thread. This code is not perfect!!!!
 */

#define DEBUG_TEST_MODE_1

#ifdef DEBUG_TEST_MODE_1
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"    
#include "error_logger.hpp"
#include "math"

using namespace PUTM;

extern StateMachine air_state_machine;
extern State error;
extern State idle;
extern State precharge;
extern State on;
extern State discharge;

extern ErrorChecker error_checker


/**
 *  @brief  Set data for all devices to default values
 *  @param  cv Cell voltage to set for all cells in all devices  
 *  @param  ovuv Over voltage under voltage to set for all devices
 *  @param  ct Cell temperature to set for all cells in all devices
 *  @param  otut Over temperature under temperature to set for all devices
 *  @param  acv Battery voltage 
 *  @param  cvt Car voltage 
 *  @param  c Current 
 *  @param  cmd Command 
 *  @param  tsms TSMS
 *  @param  err Error, mostly used for other errors not directly related to data being set in this function
 *  @return void
 *  @note   This function is used to set the data for all devices to set values. 
 *          This function is only used for testing purposes. It is not used in the final application.
 */
void set_data(float cv, bool ovuv, float ct, bool otut, float acv, float cvt, float c, bool cmd, bool tsms, bool err)
{
    /* Set data cell voltages and ovuv for all devices to default */
    for(size_t i = 0; i < Config::STACK_SIZE; i++)
    {
        for(size_t j = 0; j < Config::CELL_COUNT; j++)
        {
            data.cell_voltages[i][j] = cv;
            data.cell_ovuv[i][j] = ovuv;
        }
    }
    /* Set data cell temperatures and otut for all devices to default */
    for(size_t i = 0; i < Config::STACK_SIZE; i++)
    {
        for(size_t j = 0; j < Config::TEMPERATURES_COUNT; j++)
        {
            data.cell_temperatures[i][j] = ct;
            data.cell_otut[i][j] = otut;
        }
    }
    /* Set battery voltage to default for idle state */
    data.acu_voltage = acv;
    /* Set car voltage to default for idle state */
    data.car_voltage = cvt;
    /* Set current to default for idle state */
    data.current = c;
    /* Reset cmd */
    data.cmd_on = cmd;
    /* Reset error */
    data.error = err;
    /* Reset tsms */
    data.tsms = tsms;
}

void test_idle_sub_error(const char *fail_message)
{
    /* Force idle state */
    air_state_machine.set_current_state(&idle);
    /* Wait a bit, preferably around 300ms (the fault contition should be raised faster than 200ms) */
    tx_thread_sleep(300);
    if(air_state_machine.current_state != &error)
    {
        /* Print error message */   
        printf("ERROR: State machine failed to go to error when %s\n", fail_message);
    }
    else
    {
        /* Print success message */
        printf("MESSAGE: State machine went to error when %s\n", fail_message);
    }
}

void test_idle()
{
    /* TEST 1 */
    error_checker.reset();
    /* Set data to emulate a working condition */
    set_data(3.5f, false, 24.f, false, 550.f, 0.f, 0.f, false, false, false);
    /* Force idle state */
    air_state_machine.set_current_state(&idle);
    /* Wait a bit, preferably around 500ms */
    tx_thread_sleep(500);
    /* Check if the state machine is still in idle state */
    if(air_state_machine.current_state != &idle)
    {
        printf("ERROR: State machine failed to stay in idle\n");
    }
    else
    {
        printf("MESSAGE: State machine stayed in idle\n");
    }

    /* TEST 2 */
    error_checker.reset();
    /* Set data to emulate a cell faulty reading */
    set_data(0.f, false, 24.f, false, 550.f, 0.f, 0.f, false, false, false);
    /*       ~~~                                                          */
    test_idle_sub_error("cell voltage = 0.f");

    /* TEST 3 */
    error_checker.reset();
    /* Set data to emulate a ovuv condition */
    set_data(3.5f, true, 24.f, false, 550.f, 0.f, 0.f, false, false, false);
    /*             ~~~~                                                   */
    test_idle_sub_error("ovuv = true");

    /* TEST 4 */
    error_checker.reset();
    /* Set data to emulate a temp faulty reading */
    set_data(3.5f, false, 60.f, false, 550.f, 0.f, 0.f, false, false, false);
    /*                    ~~~~                                             */
    test_idle_sub_error("tcell = 60.f");

    /* TEST 5 */
    error_checker.reset();
    /* Set data to emulate a otut condition */
    set_data(3.5f, false, 24.f, true, 550.f, 0.f, 0.f, false, false, false);
    /*                          ~~~~                                      */
    test_idle_sub_error("otut = true");

    /* TEST 6 */
    error_checker.reset();
    /* Set data to emulate a acu faulty reading */
    set_data(3.5f, false, 24.f, false, 200.f, 0.f, 0.f, false, false, false);
    /*                                 ~~~~                                */
    test_idle_sub_error("vacu = 200.f");

    /* TEST 7 */
    error_checker.reset();
    /* Set data to emulate a car faulty reading */
    set_data(3.5f, false, 24.f, false, 550.f, 100.f, 0.f, false, false, false);
    /*                                        ~~~~~                          */
    test_idle_sub_error("vcar = 100.f");

    /* TEST 8 */
    error_checker.reset();
    /* Set data to emulate a current faulty reading */
    set_data(3.5f, false, 24.f, false, 550.f, 0.f, 100.f, false, false, false);
    /*                                             ~~~~~                     */
    test_idle_sub_error("current = 100.f");

    /* TEST 9 */
    error_checker.reset();
    /* Set data to emulate other errors */
    set_data(3.5f, false, 24.f, false, 550.f, 0.f, 0.f, false, true, false);
    /*                                                         ~~~~       */
    test_idle_sub_error("error = true");

    /* RESET */
    error_checker.reset();
    set_data(3.5f, false, 24.f, false, 550.f, 0.f, 0.f, false, false, false);
    /* Set data to emulate a tsms on and cmd on */
    // TODO: idk, a check for going to precharge state will be tricky
}

void test_precharge()
{
    float vcar = 0.f;
    uint32_t start_time = 0.f;

    /* TEST 1 */
    error_checker.reset();
    /* Set data to emulate a working condition - entering precharge */
    set_data(3.5f, false, 24.f, false, 550.f, 0.f, 0.f, true, false, true);
    /* Force idle state */
    air_state_machine.set_current_state(&idle);
    vcar = 0.f;
    start_time = tx_time_get();
    while (vcar < 550.f)
    {
        /* this should simulate rising of the voltage from 0 to 550.f for around 5s */
        float time = (float)(tx_time_get() - start_time) * 0.001f;
        vcar = std::exp(-0.2f * time) * 600.f;
        tx_thread_sleep(5);
    }
    if(air_state_machine.current_state != &on)
    {
        printf("ERROR: State machine failed to go to on\n");
    }
    else
    {
        printf("MESSAGE: State machine went to on\n");
    }

}

VOID test_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        tx_thread_sleep(100);
    }
}
#endif /* TEST_MODE_1 */
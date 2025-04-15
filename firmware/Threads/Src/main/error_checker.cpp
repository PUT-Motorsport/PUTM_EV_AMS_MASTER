#include "main.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"

/* Error handler */

Error com_error
{
    .name = "Communication error",
    .timeout
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

Error vcell_error
{
    .name = "Cell voltage error",
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

Error tcell_error
{
    .name = "Temperatures voltage error",
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

Error current_error
{
    .name = "Current error",
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

Error v_error
{
    .name = "Voltage Car/Acu error",
    .condition []() -> uint32_t 
    {
        return 0;
    }
};

ErrorCheker error_checker;

void init_error_checker(ErrorChecker *error_checker)
{
    error_checker->add_errors(
        com_error,
        vcell_error,
        tcell_error,
        current_error,
        v_error);
}
#include "main.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"

using namespace PUTM;

Error com_error
{
    .name = "Communication error",
    .timeout = Config::ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

Error vcell_error
{
    .name = "Cell voltage error",
    .timeout = Config::ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;

        return code;
    }
};

Error tcell_error
{
    .name = "Temperatures error",
    .timeout = Config::ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;

        return code;
    }
};

Error current_error
{
    .name = "Current error",
    .timeout = Config::ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.current > Config::MAX_CURRENT_THRESH)
        {
            code = 1;
        }
        else if(data.current < Config::MIN_CURRENT_THRESH)
        {
            code = 2;
        }
        return code;
    },
    .parse = [](uint32_t code) -> const char* 
    {
        switch(code)
        {
            case 1:
                return "Current too high";
            case 2:
                return "Current too low";
            default:
                return "";
        }
    }
};

Error v_error
{
    .name = "Voltage Car/Acu error",
    .timeout = Config::ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.acu_voltage > Config::MAX_BAT_VOLTAGE)
        {
            code = 1;
        }
        else if(data.acu_voltage < Config::MIN_BAT_VOLTAGE)
        {
            code = 2;
        }
        else if(data.car_voltage > Config::MAX_BAT_VOLTAGE)
        {
            code = 3;
        }
        // FIXME: this condition is not needed? it may be needed for the future
        // else if(data.car_voltage < Config::MIN_BAT_VOLTAGE)
        // {
        //     code = 4;
        // }
        return code;
    },
    .parse = [](uint32_t code) -> const char* 
    {
        switch(code)
        {
            case 1:
                return "Acu voltage too high";
            case 2:
                return "Acu voltage too low";
            case 3:
                return "Car voltage too high";
            // case 4:
            //     return "Car voltage too low";
            default:
                return "";
        }
    }
};

ErrorChecker error_checker;

void init_error_checker(ErrorChecker *ec)
{
    ec->add_errors(
        com_error,
        vcell_error,
        tcell_error,
        current_error,
        v_error);
}
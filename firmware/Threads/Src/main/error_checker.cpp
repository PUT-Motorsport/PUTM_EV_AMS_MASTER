#include "main.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"

using namespace PUTM;

extern StateMachine air_state_machine;
extern State idle;
extern State precharge;
extern State on;
extern State error;

Error com_error
{
    .name = "Communication error",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        return 0;
    }
};

static uint32_t encode_error(uint8_t dev, uint8_t cell, uint8_t error)
{
    return ((uint32_t)(dev) << 16) | ((uint32_t)(cell) << 8) | (uint32_t)(error);
}

Error vcell_error
{
    .name = "Cell voltage error",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_voltages[i * Config::STACK_SIZE + j] > Config::CELL_OV_FLOAT)
                {
                    return encode_error((uint8_t)i, (uint8_t)j, 1);
                }
                else if(data.cell_voltages[i * Config::STACK_SIZE + j] < Config::CELL_UV_FLOAT)
                {
                    return encode_error((uint8_t)i, (uint8_t)j, 2);
                }
            }
        }

        return 0;
    },
    .parse = [](uint32_t code) -> const char* 
    {
        uint8_t dev = (code >> 16) & 0xFF;
        uint8_t cell = (code >> 8) & 0xFF;
        uint8_t error = code & 0xFF;

        switch(error)
        {
            case 1:
                return "Cell overvoltage";
            case 2:
                return "Cell undervoltage";
            default:
                return "";
        }
    }
};

Error tcell_error
{
    .name = "Temperatures error",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_temperatures[i * Config::STACK_SIZE + j] > Config::CELL_OT)
                {
                    return encode_error((uint8_t)i, (uint8_t)j, 1);
                }
                else if(data.cell_temperatures[i * Config::STACK_SIZE + j] < Config::CELL_UT)
                {
                    return encode_error((uint8_t)i, (uint8_t)j, 2);
                }
            }
        }

        return 0;
    },
    .parse = [](uint32_t code) -> const char* 
    {
        uint8_t dev = (code >> 16) & 0xFF;
        uint8_t cell = (code >> 8) & 0xFF;
        uint8_t error = code & 0xFF;

        switch(error)
        {
            case 1:
                return "Temperature overtemperature";
            case 2:
                return "Temperature undertemperature";
            default:
                return "";
        }
    }
};

Error current_error
{
    .name = "Current error",
    .timeout = Config::CURRENT_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.current > Config::MAX_CURRENT_THRESH_LONG)
        {
            code = 1;
        }
        else if(data.current < Config::MIN_CURRENT_THRESH_LONG)
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
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
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
        // TODO: this condition is not needed? it may be needed for the future
        // else if(data.car_voltage < Config::MIN_BAT_VOLTAGE)
        // {
        //     code = 4;
        // }
        else if(air_state_machine.get_current_state() == &on)
        {
            if(data.car_voltage < Config::MIN_BAT_VOLTAGE)
            {
                code = 5;
            }
        }
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
            case 5:
                return "Car voltage too low while on";
            default:
                return "";
        }
    }
};

Error precharge_timeout
{
    .name = "Precharge timeout",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        return data.precharge_error;
    },
    .parse = [](uint32_t code) -> const char* 
    {
        switch(code)
        {
            case 1:
                return "Precharge timeout low";
            case 2:
                return "Precharge timeout high";
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
        //tcell_error, //FIXME: this error is not used yet
        current_error,
        v_error);
}
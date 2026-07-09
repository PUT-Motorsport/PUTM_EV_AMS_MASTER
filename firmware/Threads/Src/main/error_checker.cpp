#include "main.h"
#include "stm32h573xx.h"
#include "stm32h5xx_hal_def.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"

#include "tuple"

#include "threads.hpp"
#include "wrapper/gpio.hpp"
#include "data.hpp"
#include "config.hpp"
#include "state_machine.hpp"
#include "error_checker.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <cstdint>

using namespace PUTM;
using namespace Utils;

static char error_write_buffer[128];

extern StateMachine air_state_machine;
extern State idle;
extern State precharge;
extern State on;
extern State error;

ErrorChecker error_checker;

Logger<1024 * 2> error_logger;

static uint32_t encode_error(uint8_t dev, uint8_t unit, uint8_t error)
{
    return ((uint32_t)(dev) * 10'000) + ((uint32_t)(unit) * 100) + (uint32_t)(error);
}

static std::tuple<uint8_t, uint8_t, uint8_t> decode_error(uint32_t code)
{
    return { code / 10'000 % 100, code / 100 % 100, code % 100 };
}

void log_com_error(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, unit, error] = decode_error(code);
    if (error == 0) return;
    if(dev == 0 and unit == 0)
    {
        error_logger.log_error("COM\0", tx_time_get());
    }
    else
    {
        std::format_to_n(error_write_buffer, sizeof(error_write_buffer), "COM: D {}", dev, unit);
        error_logger.log_error(error_write_buffer, tx_time_get());
    }
}

Error com_error
{
    .name = "E: COM",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.bq_init_status != HAL_OK) 
        {
            code = encode_error(0, 0, 1);
        }
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            if(data.bq_read_data_status[i] != HAL_OK)
            {
                code = encode_error(i + 1, 0, data.bq_read_data_status[i]);
            }
        }
        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_com_error(code);
        // error->reset();
    }
};

void log_vcell_error(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, cell, error] = decode_error(code);
    if (error == 0) return;
    std::format_to_n(error_write_buffer, sizeof(error_write_buffer), "CELL V: D {}, C {}", dev, cell);
    error_logger.log_error(error_write_buffer, tx_time_get());
}

Error vcell_error
{
    .name = "E: CELL V",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::CELL_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_voltages[i][j] > Config::CELL_OV_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 1);
                }
                else if(data.cell_voltages[i][j] < Config::CELL_UV_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 2);
                }
            }
        }

        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_vcell_error(code);
        // error->reset();
    }
};

void log_tcell_error(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, temp, error] = decode_error(code);
    if (error == 0) return;
    std::format_to_n(error_write_buffer, sizeof(error_write_buffer), "CELL T: D {}, T {}", dev, temp);
    error_logger.log_error(error_write_buffer, tx_time_get());
}

Error tcell_error
{
    .name = "E: TEMP",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        for(size_t i = 0; i < Config::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < Config::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_temperatures[i][j] > Config::CELL_OT_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 1);
                }
                else if(data.cell_temperatures[i][j] < Config::CELL_UT_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 2);
                }
            }
        }

        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_tcell_error(code);
        // error->reset();
    }
};

void log_current_error(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, unit, error] = decode_error(code);
    if (error == 0) return;
    error_logger.log_error("I LONG", tx_time_get());
}

Error current_error_long
{
    .name = "E: CURRENT LONG",
    .timeout = Config::CURRENT_ERROR_LONG_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.current > Config::MAX_CURRENT_THRESH_LONG)
        {
            code = encode_error(0, 0, 1);
        }
        else if(data.current < Config::MIN_CURRENT_THRESH_LONG)
        {
            code = encode_error(0, 0, 2);
        }
        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_current_error(code);
        // error->reset();
    }
};

void log_current_error_short(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, unit, error] = decode_error(code);
    if (error == 0) return;
    error_logger.log_error("I SHORT", tx_time_get());
}

Error current_error_short
{
    .name = "E: CURRENT SHORT",
    .timeout = Config::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.current > Config::MAX_CURRENT_THRESH_SHORT)
        {
            code = 1;
        }
        else if(data.current < Config::MIN_CURRENT_THRESH_SHORT)
        {
            code = encode_error(0, 0, 2);
        }
        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_current_error_short(code);
        // error->reset();
    }
};

void log_v_error(uint32_t code)
{
    static uint32_t last_code { 0 };
    if (last_code == code) return; // avoid logging the same error multiple times
    last_code = code;
    auto [dev, unit, error] = decode_error(code);
    if (error == 0) return;
    std::format_to_n(error_write_buffer, sizeof(error_write_buffer), "TS ERR: D {}", error);
    error_logger.log_error("TS ERR", tx_time_get());
}

Error v_error
{
    .name = "E: TS VOLTAGE",
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
        else if(data.cell_voltage_sum > Config::MAX_BAT_VOLTAGE)
        {
            code = 3;
        }
        // else if(data.car_voltage > Config::MAX_BAT_VOLTAGE)
        // {
        //     code = 3;
        // }
        // TODO: this condition is not needed? it may be needed for the future
        // else if(data.car_voltage < Config::MIN_BAT_VOLTAGE)
        // {
        //     code = 4;
        // }
        // else if(air_state_machine.get_current_state() == &on)
        // {
        //     if(data.car_voltage < Config::MIN_BAT_VOLTAGE)
        //     {
        //         code = 5;
        //     }
        // }
        return code;
    },
    .callback = [](Error* error, uint32_t code)
    {
        log_v_error(code);
        // error->reset();
    }
};

// Error precharge_timeout
// {
//     .name = "E: PRE SLOW",
//     .timeout = Config::STANDARD_ERROR_TIMEOUT,
//     .condition = []() -> uint32_t 
//     {
//         return data.precharge_error;
//     },
// };

void init_error_checker(ErrorChecker *ec)
{
    ec->add_errors(
        com_error,
        vcell_error,
        tcell_error,
        current_error_long,
        current_error_short,
        v_error
    );
}
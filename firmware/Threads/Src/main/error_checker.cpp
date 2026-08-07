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
        data.loggers.errors.log_error("COM\0", tx_time_get());
    }
    else
    {
        std::format_to_n(error_write_buffer, sizeof(error_write_buffer), "COM: D {}", dev, unit);
        data.loggers.errors.log_error(error_write_buffer, tx_time_get());
    }
}

Error com_error
{
    .name = "E: COM",
    .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.bq_init_status != HAL_OK) 
        {
            code = encode_error(0, 0, 1);
        }
        for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
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
    data.loggers.errors.log_error(error_write_buffer, tx_time_get());
}

Error vcell_error
{
    .name = "E: CELL V",
    .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
        {
            for(size_t j = 0; j < CONFIG::CELL_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_voltages[i][j] > CONFIG::CELL_OV_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 1);
                }
                else if(data.cell_voltages[i][j] < CONFIG::CELL_UV_FLOAT)
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
    data.loggers.errors.log_error(error_write_buffer, tx_time_get());
}

Error tcell_error
{
    .name = "E: TEMP",
    .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        if constexpr (CONFIG::TURN_OFF_TEMP_ERRORS) return 0;
        uint32_t code = 0;
        for(size_t i = 0; i < CONFIG::STACK_SIZE; i++)
        {
            size_t errors_per_stack = 0;
            for(size_t j = 0; j < CONFIG::TEMPERATURES_COUNT_PER_DEVICE; j++)
            {
                if(data.cell_temperatures[i][j] > CONFIG::CELL_OT_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 1);
                    errors_per_stack++;
                }
                else if(data.cell_temperatures[i][j] < CONFIG::CELL_UT_FLOAT)
                {
                    code = encode_error(i + 1, j + 1, 2);
                    errors_per_stack++;
                }
            }
            if(errors_per_stack > CONFIG::MAX_IGNORABLE_TEMPERATURES)
            {
                return code;
            }
        }

        return 0;
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
    data.loggers.errors.log_error("I LONG", tx_time_get());
}

Error current_error_long
{
    .name = "E: CURRENT LONG",
    .timeout = CONFIG::CURRENT_ERROR_LONG_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        if constexpr (CONFIG::TURN_OFF_TEMP_ERRORS) return 0;
        uint32_t code = 0;
        if(data.current > CONFIG::MAX_CURRENT_THRESH_LONG)
        {
            code = encode_error(0, 0, 1);
        }
        else if(data.current < CONFIG::MIN_CURRENT_THRESH_LONG)
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
    data.loggers.errors.log_error("I SHORT", tx_time_get());
}

Error current_error_short
{
    .name = "E: CURRENT SHORT",
    .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.current > CONFIG::MAX_CURRENT_THRESH_SHORT)
        {
            code = 1;
        }
        else if(data.current < CONFIG::MIN_CURRENT_THRESH_SHORT)
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
    data.loggers.errors.log_error("TS ERR", tx_time_get());
}

Error v_error
{
    .name = "E: TS VOLTAGE",
    .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
    .condition = []() -> uint32_t 
    {
        uint32_t code = 0;
        if(data.acu_voltage > CONFIG::MAX_BAT_VOLTAGE)
        {
            code = 1;
        }
        else if(data.acu_voltage < CONFIG::MIN_BAT_VOLTAGE)
        {
            code = 2;
        }
        else if(data.cell_voltage_sum > CONFIG::MAX_BAT_VOLTAGE)
        {
            code = 3;
        }
        // else if(data.car_voltage > CONFIG::MAX_BAT_VOLTAGE)
        // {
        //     code = 3;
        // }
        // TODO: this condition is not needed? it may be needed for the future
        // else if(data.car_voltage < CONFIG::MIN_BAT_VOLTAGE)
        // {
        //     code = 4;
        // }
        // else if(air_state_machine.get_current_state() == &on)
        // {
        //     if(data.car_voltage < CONFIG::MIN_BAT_VOLTAGE)
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
//     .timeout = CONFIG::STANDARD_ERROR_TIMEOUT,
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
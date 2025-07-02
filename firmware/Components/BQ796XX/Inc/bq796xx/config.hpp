#pragma once

#include "array"

#include "tx_api.h"
#include "main.h"
#include "usart.h"

#include "utils.hpp"
#include "config.hpp"
#include "wrapper/uart.hpp"

namespace PUTM
{
    namespace Config
    {    
        /* Bq796xx configuration namespace */
        
        /* Bq79600 has a preset baud rate of 1Mbps [bps] */
        constexpr static inline uint32_t DEFAULT_BAUDRATE = 1'000'000;
        /* Time hold wakeup, in range <2500, 3000> [us] */
        constexpr static inline uint32_t T_WAKEUP = 2'600;
        /* Rx timeout [us] (2 * defualt) */
        constexpr static inline uint32_t T_RX_TIMEOUT = 500;
        /* Baudrate for init [bps], assume 1 bit high before, 6 bits of low time and 1 bit high after, this should create the required pattern */
        constexpr static inline uint32_t BAUDRATE_WAKEUP = (uint32_t)(1'000'000.0 / (double)T_WAKEUP * 6.0);
        /* Rx timeout represented in baudblocks */
        constexpr static inline uint32_t RX_TIMEOUT_BAUDBLOCKS = (uint32_t)((double)T_RX_TIMEOUT * (double)DEFAULT_BAUDRATE / 1'000'000.0);
        /* uV per bit */
        constexpr static inline double V_LSB_ADC = 190.73e-6;
    }
}
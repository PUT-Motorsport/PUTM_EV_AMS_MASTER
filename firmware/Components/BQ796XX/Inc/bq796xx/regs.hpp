#pragma once

#include "main.h"

#include "utils.hpp"

namespace PUTM
{
    namespace Bq796xx
    {
        namespace Types
        {   
            enum struct ScanMode : uint8_t
            {
                Stop,
                RoundRobin,
                Once,
                SingleChannel
            };

            enum struct GpioMode : uint8_t
            {
                HighZ,
                AdcOtut
            }
        }

        namespace Regs
        {
            struct __packed Control1 : public Utils::IReg<0x0309>
            {
                bool addr_wr : 1 { 0b0 };
                bool soft_reset : 1 { 0b0 };
                bool goto_sleep : 1 { 0b0 };
                bool goto_shutdown : 1 { 0b0 };
                bool send_slptoact : 1 { 0b0 };
                bool send_wake : 1 { 0b0 };
                bool send_shutdown : 1 { 0b0 };
                bool dir_sel : 1 { 0b0 };
            };

            struct __packed AdcCtrl1 : public Utils::IReg<0x030d>
            {
                Types::ScanMode main_mode : 2 { Types::ScanMode::Stop };
                bool main_go : 1 { 0b0 };
                bool lpf_cell_en : 1 { 0b0 };
                bool lpf_bb_en : 1 { 0b0 };
                uint8_t reserved : 3 { 0b000 };
            };
    
            struct __packed OVUVCtrl : public Utils::IReg<0x032C>
            {
                Types::ScanMode ovuv_mode : 2 { Types::Mode::Stop };
                bool ovuv_go : 1 { false };
                uint8_t ovuv_lock : 4 { 0 };
                uint8_t vcbdone_thr_lock : 1 { 0 };
            };

            struct __packed GPIOConf1 : public Utils::IReg<0x000E>
            {
                GpioMode gpio1 : 3 { GpioMode::HighZ };
                GpioMode gpio2 : 3 { GpioMode::HighZ };
                bool spi_en : 1 { false };
                bool fault_in_en : 1 { false };
            };

            struct __packed GPIOConf2 : public Utils::IReg<0x000F>
            {
                GpioMode gpio3 : 3 { GpioMode::HighZ };
                GpioMode gpio4 : 3 { GpioMode::HighZ };
                bool reserved : 1 { false };
                bool spare : 1 { false };
            };

            struct __packed GPIOConf3 : public Utils::IReg<0x0010>
            {
                GpioMode gpio5 : 3 { GpioMode::HighZ };
                GpioMode gpio6 : 3 { GpioMode::HighZ };
                uint8_t spare : 2 { 0x00 };
            };

            struct __packed GPIOConf4 : public Utils::IReg<0x0011>
            {
                GpioMode gpio7 : 3 { GpioMode::HighZ };
                GpioMode gpio8 : 3 { GpioMode::HighZ };
                uint8_t spare : 2 { 0x00 };
            };
        }
    }
}
#pragma once

#include "main.h"

#include "utils.hpp"

namespace PUTM
{
    namespace Bq796xx
    {
        namespace Types
        {   
            enum struct Mode : uint8_t
            {
                Stop,
                RoundRobin,
                Once,
                SingleChannel
            };
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
                Types::Mode main_mode : 2 { Types::Mode::Stop };
                bool main_go : 1 { 0b0 };
                bool lpf_cell_en : 1 { 0b0 };
                bool lpf_bb_en : 1 { 0b0 };
                uint8_t reserved : 3 { 0b000 };
            };
    
            struct __packed OVUVCtrl : public Utils::IReg<0x032C>
            {
                Types::Mode ovuv_mode : 2 { Types::Mode::Stop };
                bool ovuv_go : 1 { false };
                uint8_t ovuv_lock : 4 { 0 };
                uint8_t vcbdone_thr_lock : 1 { 0 };
            };
        }
    }
}
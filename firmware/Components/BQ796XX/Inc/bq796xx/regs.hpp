#pragma once

#include "main.h"

#include "utils.hpp"
#include <cstdint>

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

            enum struct ScanMode2 : uint8_t
            {
                Stop,
                Single,
                RoundRobin
            };

            enum struct GpioMode : uint8_t
            {
                HighZ,
                AdcOtut
            };

            enum struct TwarnThr : uint8_t
            {
                _85degC,
                _95degC,
                _105degC,
                _115degC
            };

            enum struct SlpTime : uint8_t
            {
                Disable,
                _5s,
                _10s,
                _1min,
                _10min,
                _30min,
                _1h,
                _2h
            };

            enum struct CtlAct : uint8_t
            {
                GoToSleep,
                GoToShutdown
            };

            enum struct CtlTime : uint8_t 
            {
                Disable,
                _100ms,
                _2s,
                _10s,
                _1min, // default
                _10min,
                _30min,
                _1h
            };

            typedef CtlTime CtsTime;
        }

        namespace Regs
        {
            struct __packed BalCtrl2 : public Utils::IReg<0x032F>
            {
                bool auto_bal : 1 { false };
                bool bal_go : 1 { false };
                uint8_t bal_act : 2 { 0b00 };
                bool otcb_en : 1 { false };
                bool fltstop_en : 1 { false };
                bool cb_pause : 1 { false };
                uint8_t rsvd : 1 { 0b0 };
            };
             
            struct __packed Control1 : public Utils::IReg<0x0309>
            {
                bool addr_wr : 1 { false };
                bool soft_reset : 1 { false };
                bool goto_sleep : 1 { false };
                bool goto_shutdown : 1 { false };
                bool send_slptoact : 1 { false };
                bool send_wake : 1 { false };
                bool send_shutdown : 1 { false };
                bool dir_sel : 1 { false };
            };

            struct __packed Control2 : public Utils::IReg<0x030A>
            {
                bool tsref_en : 1 { false };
                bool send_hw_reset : 1 { false };
                uint8_t rsvd : 6 { 0b000000 };
            };

            struct __packed AdcCtrl1 : public Utils::IReg<0x030d>
            {
                Types::ScanMode2 main_mode : 2 { Types::ScanMode2::Stop };
                bool main_go : 1 { 0b0 };
                bool lpf_cell_en : 1 { 0b0 };
                bool lpf_bb_en : 1 { 0b0 };
                uint8_t reserved : 3 { 0b000 };
            };
    
            struct __packed OVUVCtrl : public Utils::IReg<0x032C>
            {
                Types::ScanMode ovuv_mode : 2 { Types::ScanMode::Stop };
                bool ovuv_go : 1 { false };
                uint8_t ovuv_lock : 4 { 0 };
                uint8_t vcbdone_thr_lock : 1 { 0 };
            };

            struct __packed GPIOConf1 : public Utils::IReg<0x000E>
            {
                Types::GpioMode gpio1 : 3 { Types::GpioMode::HighZ };
                Types::GpioMode gpio2 : 3 { Types::GpioMode::HighZ };
                bool spi_en : 1 { false };
                bool fault_in_en : 1 { false };
            };

            struct __packed GPIOConf2 : public Utils::IReg<0x000F>
            {
                Types::GpioMode gpio3 : 3 { Types::GpioMode::HighZ };
                Types::GpioMode gpio4 : 3 { Types::GpioMode::HighZ };
                bool reserved : 1 { false };
                bool spare : 1 { false };
            };

            struct __packed GPIOConf3 : public Utils::IReg<0x0010>
            {
                Types::GpioMode gpio5 : 3 { Types::GpioMode::HighZ };
                Types::GpioMode gpio6 : 3 { Types::GpioMode::HighZ };
                uint8_t spare : 2 { 0x00 };
            };

            struct __packed GPIOConf4 : public Utils::IReg<0x0011>
            {
                Types::GpioMode gpio7 : 3 { Types::GpioMode::HighZ };
                Types::GpioMode gpio8 : 3 { Types::GpioMode::HighZ };
                uint8_t spare : 2 { 0x00 };
            };

            struct __packed OTUTThresh : public Utils::IReg<0x000B>
            {
                uint8_t ot_thr : 5 { 0x00 };
                uint8_t ut_thr : 3 { 0x00 };
            };

            struct __packed OTUTCtrl : public Utils::IReg<0x032D>
            {
                Types::ScanMode otut_mode : 2 { Types::ScanMode::Stop };
                bool otut_go : 1 { false };
                uint8_t otut_lock : 3 { 0b000 };
                uint8_t vcbdone_thr_lock : 1 { 0b0 };
                uint8_t reserved : 1 { 0b0 };
            };

            struct __packed PwrTransitConf : public Utils::IReg<0x0018>
            {
                Types::SlpTime slp_time : 3 { Types::SlpTime::Disable };
                Types::TwarnThr twarn_thr : 2 { Types::TwarnThr::_85degC };
                uint8_t spare : 3 { 0b000 };
            };

            struct __packed CommTimeoutConf : public Utils::IReg<0x0019>
            {
                Types::CtlTime ctl_time : 3 { Types::CtlTime::Disable };
                Types::CtlAct ctl_act : 1 { Types::CtlAct::GoToSleep };
                Types::CtsTime cts_time : 3 { Types::CtsTime::Disable };
                uint8_t spare : 1 { 0b0 };
            };
        }
    }
}
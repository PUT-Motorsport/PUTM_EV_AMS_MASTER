#pragma once

#include "utils.hpp"
#include "ads131m04/types.hpp"

namespace PUTM
{
    namespace Ads131m04
    {
        namespace Regs
        {
            struct __packed Id : public Utils::IReg<0x00>
            {
                const uint16_t reserved0: 8  { 0b000000000 };
                const uint16_t chancnt: 4  { 0b0000 };
                const uint16_t reserved1: 4  { 0b0000 };
            };

            struct __packed Status : public Utils::IReg<0x01>
            {
                bool drdy0: 1 { 0b0 };
                bool drdy1: 1 { 0b0 };
                bool drdy2: 1 { 0b0 };
                bool drdy3: 1 { 0b0 };
                uint16_t reserved0: 4  { 0b0000 };
                uint16_t wlength: 2 { 0b0 };
                bool reset: 1 { 0b0 };
                Types::CrcType crc_type: 1 { 0b0 };
                bool crc_err: 1 { 0b0 };
                bool reg_map: 1 { 0b0 };
                bool f_resync: 1 { 0b0 };
                bool lock: 1 { 0b0 };
            };

            struct __packed Mode : public Utils::IReg<0x02>
            {
                bool drdy_fmt: 1;
                bool drdy_hiz: 1;
                uint16_t drdy_sel: 2;
                bool timeout: 1;
                const uint16_t reserved0: 3 = { 0b000 };
                uint16_t wlength: 2;
                bool reset: 1;
                Types::CrcType crc_type: 1;
                bool rx_crc_en: 1;
                bool reg_crc_en: 1;
                const uint16_t reserved1: 2 = { 0b00 };
            };

            struct __packed Clock : public Utils::IReg<0x03>
            {
                Types::Pwr pwr : 2 { 0b00 };
                Types::Osr osr : 3 { 0b000 };
                bool tbm : 1 { 0b0 };
                uint16_t reserved0 : 2 { 0b00 };
                bool ch0_en : 1 { 0b0 };
                bool ch1_en : 1 { 0b0 };
                bool ch2_en : 1 { 0b0 };
                bool ch3_en : 1 { 0b0 };
                uint16_t reserved1 : 4 { 0b0000 };
            };

            struct __packed Gain : public Utils::IReg<0x04>
            {
                Types::PgaGain pga_gain0 : 3 { 0b000 };
                uint16_t reserved0: 1 { 0b0 };
                Types::PgaGain pga_gain1 : 3 { 0b000 };
                uint16_t reserved1: 1 { 0b0 };
                Types::PgaGain pga_gain2 : 3 { 0b000 };
                uint16_t reserved2: 1 { 0b0 };
                Types::PgaGain pga_gain3 : 3 { 0b000 };
                uint16_t reserved3: 1 { 0b0 };
            };

            struct __packed Cfg : public Utils::IReg<0x06>
            {
                bool cd_en: 1 { 0b0 };
                Types::CdLen cd_len: 3 { 0b000 };
                Types::CdNum cd_num : 3 { 0b000 };
                bool cd_allch: 1 { 0b0 };
                bool gc_en: 1 { 0b0 };
                Types::GcDly gc_dly: 4 { 0b000 };
                uint16_t reserved0: 3 { 0b000 };
            };

            struct __packed ThrshldMsb : public Utils::IReg<0x07>
            {
                uint16_t cd_th_msb;
            };

            struct __packed ThrshldLsb : public Utils::IReg<0x8>
            {
                Types::DcBlock dc_block : 4 { 0b0000 };
                const uint16_t reserved0 : 4 { 0b0000 };
                uint16_t cd_th_lsb: 8 { 0x0 };
            };

            template<size_t T>
            concept IsChannel = T <= 3 and T >= 0;

            /* for channels 0-3 */
            template<size_t CH> requires IsChannel<CH>
            struct __packed ChCfg : public Utils::IReg<0x09, CH * 5>
            {
                Types::Mux mux: 2 { 0b00 };
                bool dcblk_dis: 1 { 0b0 };
                uint16_t reserved: 3 { 0b000 };
                uint16_t phase: 10 { 0x00 };
            };

            /* for channels 0-3 */
            template<size_t CH> requires IsChannel<CH>
            struct __packed ChOcalMsb : public Utils::IReg<0x0a, CH * 5>
            {
                uint16_t ocal_msb;
            };

            /* for channels 0-3 */
            template<size_t CH> requires IsChannel<CH>
            struct __packed ChOcalLsb : public Utils::IReg<0x0b, CH * 5>
            {
                uint16_t ocal_lsb: 8 { 0x0 };
                uint16_t reserved0: 8 { 0x0 };
            };

            /* for channels 0-3 */
            template<size_t CH> requires IsChannel<CH>
            struct __packed ChGcalMsb : public Utils::IReg<0x0c, CH * 5>
            {
                uint16_t gcal_msb { 0x00 };
            };

            /* for channels 0-3 */
            template<size_t CH> requires IsChannel<CH>
            struct __packed ChGcalLsb : public Utils::IReg<0x0d, CH * 5>
            {
                uint16_t gcal_lsb: 8 { 0x0 };
                uint16_t reserved0: 8 { 0x0 };
            };

            struct __packedRegMapCrc : public Utils::IReg<0x3e>
            {
                uint16_t reg_crc { 0x00 };
            };
        }
    }
}
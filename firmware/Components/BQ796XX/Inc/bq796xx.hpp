#pragma once

#include "array"

#include "stm32h5xx_hal_def.h"
#include "tx_api.h"
#include "main.h"
#include "usart.h"

#include "utils.hpp"
#include "config.hpp"
#include "wrapper/uart.hpp"
#include "bq796xx/crc16ibm.hpp"
#include "bq796xx/regs.hpp"
#include "bq796xx/config.hpp"

namespace PUTM
{
    namespace Bq796xx
    {
        class Device
        {
        private:
            Uart *uart;
        public:
            /**
            *	@brief Init Bq796xx, set receiver timeout to t_rx_timeout (~300 us) for the uart handler to for data in IT or DMA mode
            *	@param `huart` uart handle
            */
            Device(Uart *uart) : uart(uart) { }
        public:
            /**
            * 	@brief 	Wake up function for BQ79600 IC, this functions tries to hold the UART RX line
            *			for aprox ~2.5ms
            * 	@retval	HAL_BUSY when wakeing up is in progress, HAL_OK when done
            */
            HAL_StatusTypeDef wake_up()
            {
                /* uart sends lsb first, this sequence includes start bit for a total of '6' bits */
                uint8_t out[1] { 0b1110'0000 };
                if(uart->set_baudrate(Config::BAUDRATE_WAKEUP) != HAL_OK) return HAL_ERROR;
                if(uart->await_tx_dma(out, 1) != HAL_OK) return HAL_ERROR;
                if(uart->set_baudrate(Config::DEFAULT_BAUDRATE) != HAL_OK) return HAL_ERROR;

                return HAL_OK;
            }
        private:
            enum class CommunicationMode : uint8_t
            {
                Single = 0b00000000,
                Stack = 0b00100000,
                Broadcast = 0b01000000
            };
        private:
            template<CommunicationMode COMMUNICATION_TYPE, size_t DATA_COUNT> requires (DATA_COUNT <= 8 and DATA_COUNT >= 1)
            HAL_StatusTypeDef write(uint8_t (&data)[DATA_COUNT], uint16_t reg_address, uint8_t dev_address = 1)
            {
                using enum CommunicationMode;

                size_t i = 0;
                uint8_t tx_buffer[DATA_COUNT + 7] { 0 };

                tx_buffer[i++] = 0b1'001'0000 | (uint8_t)(COMMUNICATION_TYPE) | (DATA_COUNT - 1);
                if constexpr(COMMUNICATION_TYPE == Single) tx_buffer[i++] = dev_address;
                tx_buffer[i++] = (uint8_t)(reg_address >> 8);
                tx_buffer[i++] = (uint8_t)(reg_address);
                std::copy(data, data + DATA_COUNT, tx_buffer + i);
                i += DATA_COUNT;
                uint16_t crc = Crc16::fast(tx_buffer, i);
                tx_buffer[i++] = (uint8_t)(crc >> 8);
                tx_buffer[i++] = (uint8_t)(crc);

                return uart->await_tx_dma(tx_buffer, i, Config::STACK_COM_TIMEOUT);
            }
            template<CommunicationMode COMMUNICATION_TYPE>
            HAL_StatusTypeDef write(uint8_t data, uint16_t reg_address, uint8_t dev_address = 1)
            {
                using enum CommunicationMode;

                size_t i = 0;
                uint8_t tx_buffer[8] { 0 };

                tx_buffer[i++] = 0b1'001'0000 | (uint8_t)(COMMUNICATION_TYPE);
                if constexpr(COMMUNICATION_TYPE == Single) tx_buffer[i++] = dev_address;
                tx_buffer[i++] = (uint8_t)(reg_address >> 8);
                tx_buffer[i++] = (uint8_t)(reg_address);
                tx_buffer[i++] = data;
                uint16_t crc = Crc16::fast(tx_buffer, i);
                tx_buffer[i++] = (uint8_t)(crc >> 8);
                tx_buffer[i++] = (uint8_t)(crc);

                return uart->await_tx_dma(tx_buffer, i, Config::STACK_COM_TIMEOUT);
            }
        private:
            template
            <
                CommunicationMode COMMUNICATION_TYPE, 
                size_t READ_RESITER_COUNT = 1, 
                size_t DEVICE_COUNT
            >
            requires ((COMMUNICATION_TYPE == CommunicationMode::Single and DEVICE_COUNT == 1) or
                      (COMMUNICATION_TYPE != CommunicationMode::Single))
            HAL_StatusTypeDef read(uint8_t (&data)[DEVICE_COUNT][READ_RESITER_COUNT], uint16_t reg_address, uint8_t dev_address = 1)
            {
                using enum CommunicationMode;
                constexpr size_t READ_DATA_COUNT = DEVICE_COUNT * (READ_RESITER_COUNT + 6);

                size_t i = 0;
                uint8_t tx_buffer[7] { 0 };
                uint8_t rx_buffer[READ_DATA_COUNT] { 0 };

                tx_buffer[i++] = 0b1'000'0000 | (uint8_t)(COMMUNICATION_TYPE);
                if constexpr(COMMUNICATION_TYPE == Single) tx_buffer[i++] = dev_address;
                tx_buffer[i++] = (uint8_t)(reg_address >> 8);
                tx_buffer[i++] = (uint8_t)(reg_address);
                tx_buffer[i++] = READ_RESITER_COUNT - 1;

                uint16_t crc = Crc16::fast(tx_buffer, i);
                tx_buffer[i++] = (uint8_t)(crc >> 8);
                tx_buffer[i++] = (uint8_t)(crc);

                // if(uart.await_tx_dma(buffer, i) != HAL_OK) return HAL_ERROR;
                // if(uart.await_rx_dma((uint8_t*)data, DEVICE_COUNT * DATA_COUNT + 2) != HAL_OK) return HAL_ERROR;
                if(uart->await_tx_rx_dma(tx_buffer, i, rx_buffer, READ_DATA_COUNT, Config::STACK_COM_TIMEOUT) != HAL_OK) return HAL_ERROR;
                
                for(size_t dev = 0; dev < DEVICE_COUNT; dev++)
                {
                    size_t offset = dev * (READ_RESITER_COUNT + 6) + 4;
                    std::copy(rx_buffer + offset, 
                              rx_buffer + offset + READ_RESITER_COUNT, 
                              data[dev]);
                }

                return HAL_OK; 
            }
        public:
            /**
             *  @brief 	Init Bq796xx, this function inits whole stack communication, voltage measurement and otut/ovuv protection
             *  @retval	HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
             */
            HAL_StatusTypeDef init()
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Bq796xx::Regs;
                using namespace Bq796xx::Types;

                //uart->set_rx_timeout(Config::RX_TIMEOUT_BAUDBLOCKS);
                
                if(wake_up() != HAL_OK) return HAL_ERROR;  
                
                tx_thread_sleep(10);

                /* cmd wake up slaves */
                write<Single>(Utils::convert_to<uint8_t>((Control1){ .send_wake = true }), 
                              Utils::address_of<Control1>(), 
                              0);

                /* wait ~15ms */
                tx_thread_sleep(30);

                /* reset all devices */
                write<Stack>(Utils::convert_to<uint8_t>((Control1){ .soft_reset = true }), 
                             Utils::address_of<Control1>());

                tx_thread_sleep(30);

                /* dummy write 0x00, sync internal dlls */
                for(size_t step = 0; step < 8; step++)
                {
                    write<Broadcast>(0x00, 0x343 + step);
                }

                /* enable auto adressing */
                write<Broadcast>(0x01, 0x309);

                /* auto addressing */
                for(size_t address = 0; address <= Config::STACK_SIZE; address++)
                {
                    write<Broadcast>(address, 0x306);
                }

                /* set bq7961x as stack device */
                write<Broadcast>(0x02, 0x308);

                /* set which bq is last */
                write<Single>(0x03, 0x308, Config::STACK_SIZE);

                /* dummy read sync internal dlls */
                for(size_t step = 0; step < 8; step++)
                {
                    [[maybe_unused]] uint8_t buffer[Config::STACK_SIZE][1] { 0 };
                    read<Stack>(buffer, 0x343 + step);
                }

                /* verify */
                {
                    [[maybe_unused]] uint8_t buffer[Config::STACK_SIZE][1] { 0 };
                    read<Stack>(buffer, 0x306);  
                }

                /* init tsref first to allow it to settle */
                write<Stack>(convert_to<uint8_t>((Control2){ .tsref_en = true }), 
                             address_of<Control2>());

                /* init voltage measurement, set active cells in series */
                {
                    uint8_t actice_cells = (uint8_t)(Config::CELL_COUNT_PER_DEVICE - 6);
                    write<Stack>(actice_cells, 0x0003);
                }

                /* init gpio adc measurement, set all channels to adc mode */
                {
                    uint8_t data[4]
                    { 
                        convert_to<uint8_t>((GPIOConf1){ .gpio1 = GpioMode::AdcOtut, .gpio2 = GpioMode::AdcOtut }),
                        convert_to<uint8_t>((GPIOConf2){ .gpio3 = GpioMode::AdcOtut, .gpio4 = GpioMode::AdcOtut }),
                        convert_to<uint8_t>((GPIOConf3){ .gpio5 = GpioMode::AdcOtut, .gpio6 = GpioMode::AdcOtut }),
                        convert_to<uint8_t>((GPIOConf4){ .gpio7 = GpioMode::AdcOtut, .gpio8 = GpioMode::AdcOtut })
                    };

                    write<Stack>(data, address_of<GPIOConf1>());
                }

                /* set default cell balancing voltage to 4V */
                // write<Stack>(0x3f, 0x032A);

                /* set up auto balancing */
                {
                    BalCtrl2 bal_ctrl_2 = 
                    {
                        .auto_bal = true,
                    };
                    write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                }

                /* set balancing duty */
                // write<Stack>(0x2, 0x032e);

                /* init ovuv */
                {
                    uint32_t undervoltage = std::clamp(Config::CELL_UV, 1200ul, 3100ul);
                    // TODO: fuuuuuck
                    /* this one is complicated... for now leave it like that */
                    uint32_t overvoltage = std::clamp(Config::CELL_OV, 4175ul, 4475ul);
                    
                    uint8_t data1[2]
                    {
                        // (uint8_t)(((undervoltage - 1200) / 50) & 0x3f),
                        // (uint8_t)((((overvoltage - 4175) / 25) + 0x22) & 0x3f)
                        0x24,
                        0x23
                    };

                    /* write to ov uv threshold registers */
                    write<Stack>(data1, 0x0009);
                    
                    /* disable unused channels */
                    uint8_t data2[2] = { 0 };
                    for(size_t i = Config::CELL_COUNT_PER_DEVICE; i < 16; i++)
                    {
                        size_t index = i / 8;
                        uint8_t bit = (uint8_t)(i % 8);
                        data2[index] |= (uint8_t)(1 << bit);
                    }
                    
                    std::swap(data2[0], data2[1]);

                    write<Stack>(data2, 0x000C);

                    /* enable ovuv */
                    uint8_t data3 = convert_to<uint8_t>((OVUVCtrl){ .ovuv_mode = ScanMode::RoundRobin, .ovuv_go = true });
                    
                    /* send twice, bq requires another 'go' cmd when setting are changed */
                    write<Stack>(data3, address_of<OVUVCtrl>());
                    write<Stack>(data3, address_of<OVUVCtrl>());
                }

                /* init otut */
                {
                    uint8_t undertemperature = 70ui8; // default 70%
                    uint8_t overtemperature = 30ui8; // default 30%
                    undertemperature = std::clamp(undertemperature, 66ui8, 80ui8);
                    overtemperature = std::clamp(overtemperature, 10ui8, 39ui8);

                    undertemperature = (uint8_t)((undertemperature - 66 / 2) & 0x08);
                    overtemperature = (uint8_t)((overtemperature - 10) & 0x1f);

                    uint8_t data1 = convert_to<uint8_t>((OTUTThresh){ .ot_thr = overtemperature, .ut_thr = undertemperature });

                    /* write to ov uv threshold registers */
                    write<Stack>(data1, address_of<OTUTThresh>());

                    /* enable otut */
                    uint8_t data2 = convert_to<uint8_t>((OTUTCtrl){ .otut_mode = ScanMode::RoundRobin, .otut_go = true });

                    /* send twice, bq requires another 'go' cmd when setting are changed */
                    write<Stack>(data2, 0x032C);
                    write<Stack>(data2, 0x032C);
                }

                /* init auto shutdown mode */
                {
                    PwrTransitConf pwr_transit_conf
                    {
                        .slp_time = SlpTime::_5s,
                        .twarn_thr = TwarnThr::_85degC
                    };
                    write<Stack>(convert_to<uint8_t>(pwr_transit_conf), address_of<PwrTransitConf>());

                    CommTimeoutConf comm_timeout_conf
                    {
                        .ctl_time = CtlTime::_10s,
                        .ctl_act = CtlAct::GoToShutdown,
                        .cts_time = CtsTime::_2s
                    };
                    write<Stack>(convert_to<uint8_t>(comm_timeout_conf), address_of<CommTimeoutConf>());
                }

                /* start measurement */
                {
                    uint8_t data[1] { 0 };
                    /* set adc continous, start conversion, enable lpf */
                    data[0] = convert_to<uint8_t>((AdcCtrl1){ .main_mode = ScanMode2::RoundRobin, .main_go = true });
                    write<Stack>(data, address_of<AdcCtrl1>());
                    write<Stack>(data, address_of<AdcCtrl1>());
                }

                return HAL_OK;
            }
        
        public:
            HAL_StatusTypeDef read_stack_status(bool (&ovuv_arr)[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE], 
                                                bool (&otut_arr)[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE])
            {
                using enum CommunicationMode;

                using namespace Utils;

                /* 4 ovuv registers + 2 otut registers */
                uint8_t buffer[Config::STACK_SIZE][6] { 0 };

                read<Stack>(buffer, 0x053C);

                for(size_t idev = 0; idev < Config::STACK_SIZE; idev++)
                {
                    /* over and under voltage */
                    uint16_t ov_tmp = (uint16_t)buffer[idev][0] << 8 | (uint16_t)buffer[idev][1];
                    uint16_t uv_tmp = (uint16_t)buffer[idev][2] << 8 | (uint16_t)buffer[idev][3];
                    uint16_t ovuv_tmp = ov_tmp | uv_tmp;

                    /* over and under temperature */
                    uint8_t ot_tmp = buffer[idev][4];
                    uint8_t ut_tmp = buffer[idev][5];
                    uint8_t otut_tmp = ot_tmp | ut_tmp;

                    /* voltages status */
                    for(size_t ich = 0; ich < Config::CELL_COUNT_PER_DEVICE; ich++)
                    {
                        size_t bit_index = 1 << ich;
                        ovuv_arr[idev][ich] = (bool)(ovuv_tmp & bit_index);
                    }

                    /* temperatures status */
                    for(size_t igio = 0; igio < Config::TEMPERATURES_COUNT_PER_DEVICE; igio++)
                    {
                        size_t bit_index = 1 << igio;
                        otut_arr[idev][igio] = (bool)(otut_tmp & bit_index);
                    }
                }

                return HAL_OK;
            }
        public:
            HAL_StatusTypeDef read_stack_data(float (&voltages_arr)[Config::STACK_SIZE][Config::CELL_COUNT_PER_DEVICE], 
                                              float (&temperatures_arr)[Config::STACK_SIZE][Config::TEMPERATURES_COUNT_PER_DEVICE])
            {
                using enum CommunicationMode;

                using namespace Utils;

                constexpr size_t CELL_DATA_COUNT = Config::CELL_COUNT_PER_DEVICE * 2;
                constexpr size_t REG_OFFSET = (16 - Config::CELL_COUNT_PER_DEVICE) * 2;
                constexpr size_t TEMP_DATA_COUNT = Config::TEMPERATURES_COUNT_PER_DEVICE * 2;

                uint8_t buffer_v[Config::STACK_SIZE][CELL_DATA_COUNT] { 0 };
                
                /* read voltages, address of VCELL16_HI */
                read<Stack>(buffer_v, 0x0568 + REG_OFFSET);

                /* voltages */
                for(size_t idev = 0; idev < Config::STACK_SIZE; idev++)
                {
                    for(size_t ich = 0; ich < Config::CELL_COUNT_PER_DEVICE; ich++)
                    {
                        int16_t volt = ((uint16_t)(buffer_v[idev][ich * 2]) << 8 | (uint16_t)(buffer_v[idev][ich * 2 + 1]));
                        
                        voltages_arr[idev][Config::CELL_COUNT_PER_DEVICE - 1 - ich] = -(~volt + 1) * Config::V_LSB_ADC_CELL;
                    }
                }

                uint8_t buffer_t[Config::STACK_SIZE][TEMP_DATA_COUNT] { 0 };

                /* read temperatures, address of GPIO1_HI */
                read<Stack>(buffer_t, 0x058E);

                /* temperatures */
                for(size_t idev = 0; idev < Config::STACK_SIZE; idev++)
                {
                    for(size_t igio = 0; igio < Config::TEMPERATURES_COUNT_PER_DEVICE; igio++)
                    {
                        int16_t volt = ((uint16_t)(buffer_t[idev][igio * 2]) << 8 | (uint16_t)(buffer_t[idev][igio * 2 + 1]));
                        
                        temperatures_arr[idev][igio] = -(~volt + 1) * Config::V_LSB_ADC_GPIO;
                    }
                }

                return HAL_OK;
            }
        public:
            HAL_StatusTypeDef read_single_data(float (&voltages_arr)[Config::CELL_COUNT_PER_DEVICE], 
                                               float (&temperatures_arr)[Config::TEMPERATURES_COUNT_PER_DEVICE],
                                               uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;

                constexpr size_t CELL_DATA_COUNT = Config::CELL_COUNT_PER_DEVICE * 2;
                constexpr size_t REG_OFFSET = (16 - Config::CELL_COUNT_PER_DEVICE) * 2;
                constexpr size_t TEMP_DATA_COUNT = Config::TEMPERATURES_COUNT_PER_DEVICE * 2;
                
                uint8_t buffer_v[1][CELL_DATA_COUNT] { 0 };
                
                /* read voltages, address of VCELL16_HI */
                read<Single>(buffer_v, 0x0568 + REG_OFFSET, device);

                for(size_t ich = 0; ich < Config::CELL_COUNT_PER_DEVICE; ich++)
                {
                    int16_t volt = ((uint16_t)(buffer_v[0][ich * 2]) << 8 | (uint16_t)(buffer_v[0][ich * 2 + 1]));
                    
                    voltages_arr[Config::CELL_COUNT_PER_DEVICE - 1 - ich] = -(~volt + 1) * Config::V_LSB_ADC_CELL;
                }

                uint8_t buffer_t[1][TEMP_DATA_COUNT] { 0 };

                /* read temperatures, address of GPIO1_HI */
                read<Single>(buffer_t, 0x058E, device);

                /* temperatures */

                for(size_t igio = 0; igio < Config::TEMPERATURES_COUNT_PER_DEVICE; igio++)
                {
                    int16_t volt = ((uint16_t)(buffer_t[0][igio * 2]) << 8 | (uint16_t)(buffer_t[0][igio * 2 + 1]));
                    
                    temperatures_arr[igio] = -(~volt + 1) * Config::V_LSB_ADC_GPIO;
                }

                return HAL_OK;
            }
        public:
            /**
             *  @brief 	Pause balancing for the given device
             */
            HAL_StatusTypeDef pause_balancing(uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                if(device >= Config::STACK_SIZE) return HAL_ERROR;
                if(device == 0) return HAL_ERROR;

                BalCtrl2 bal_ctrl_2
                {
                    
                    .auto_bal = true,
                    .bal_go = true,
                    .cb_pause = true
                };
                write<Single>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>(), device);

                return HAL_OK;
            }
            /**
             *  @brief 	Resume balancing for the given device
             */
            HAL_StatusTypeDef resume_balancing(uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                if(device >= Config::STACK_SIZE) return HAL_ERROR;
                if(device == 0) return HAL_ERROR;

                {
                    BalCtrl2 bal_ctrl_2
                    {
                        .auto_bal = false,
                        .bal_go = true,
                        .cb_pause = false
                    };
                    // write<Stack>(0x03, address_of<BalCtrl2>());
                    write<Single>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>(), device);
                }

                return HAL_OK;
            }
            HAL_StatusTypeDef start_balancing()
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                BalCtrl2 bal_ctrl_2
                {
                    .auto_bal = false,
                    .bal_go = true,
                    .cb_pause = false
                };
                write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());

                return HAL_OK;
            }
            HAL_StatusTypeDef start_balancing(uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                BalCtrl2 bal_ctrl_2
                {
                    .auto_bal = false,
                    .bal_go = true,
                    .cb_pause = false
                };
                write<Single>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>(), device);
                write<Single>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>(), device);

                return HAL_OK;
            }
        public:
            /**
             *  @brief 	Balance cells, this function will set balancing time to 0 and start balancing
             *          for the given cells, it will also set balancing time to 60s
             *  @param  balance_arr array of balancing cells, 0 = do not balance, 1 = balance
             *  @param  device device address, 0 = comm device, 1 = first device, 2 = second device, etc.
             *  @retval	HAL_OK when done, HAL_BUSY when balancing in progress, HAL_ERROR on fail
             */
            HAL_StatusTypeDef set_balancing(bool (&balance_arr)[Config::CELL_COUNT_PER_DEVICE], 
                                                     uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;
                
                if(device > Config::STACK_SIZE) return HAL_ERROR;
                if(device == 0) return HAL_ERROR;

                if(balance_arr == nullptr) return HAL_ERROR;

                /* return error if more than two consecutive cells are being balance at the same time */
                size_t consecutive_cells = 0;
                for(size_t i = 0; i < Config::CELL_COUNT_PER_DEVICE; i++)
                {
                    if(balance_arr[i])
                    {
                        consecutive_cells++;
                        if(consecutive_cells > 2) return HAL_ERROR; // more than two consecutive cells
                    }
                    else
                    {
                        consecutive_cells = 0;
                    }
                }
                /* return error if more than 8 cells are being balance at the same time */
                size_t balancing_cells = 0;
                for(size_t i = 0; i < Config::CELL_COUNT_PER_DEVICE; i++)
                {
                    if(balance_arr[i])
                    {
                        balancing_cells++;
                        if(balancing_cells > 8) return HAL_ERROR; // more than 8 cells
                    }
                }
                if(balancing_cells == 0) return HAL_ERROR; // no cells to balance
                
                // if(Config::CELL_COUNT_PER_DEVICE > 0)
                /* set balancing time to 10s */
                {
                    uint8_t data[8] { };
                    for(size_t i = 0; i < 8; i++)
                    {
                        size_t cell_index = 8 - 1 - i;
                        data[i] = uint8_t(balance_arr[cell_index]) * 0x1;
                    }
                    write<Single>(data, 0x0318 + 0x0008, device);
                }
                {
                    uint8_t data[Config::CELL_COUNT_PER_DEVICE - 8] { };
                    for(size_t i = 0; i < Config::CELL_COUNT_PER_DEVICE - 8; i++)
                    {
                        size_t cell_index = Config::CELL_COUNT_PER_DEVICE - 1 - i;
                        data[i] = uint8_t(balance_arr[cell_index]) * 0x1; 
                    }
                    write<Single>(data, 0x0318 + Config::CELL_COUNT_PER_DEVICE - 0x0008, device);
                }

                return HAL_OK;
            }
        public:
            HAL_StatusTypeDef read_balancing_status(uint8_t &status,
                                                    uint8_t device)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                if(device >= Config::STACK_SIZE) return HAL_ERROR;
                if(device == 0) return HAL_ERROR;

                /* read balancing status */
                uint8_t data[1][1] { 0 };
                read<Single>(data, 0x052B, device);
                status = data[0][0];

                return HAL_OK;
            }
        public:
            HAL_StatusTypeDef start_auto_balancing()
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                /* set balancing time to max */
                {
                    // if(Config::CELL_COUNT_PER_DEVICE > 0)
                    {
                        // TODO: for now leave it at 8
                        constexpr size_t data1_size = 8;
                        uint8_t data1[data1_size] { };
                        std::fill(data1, data1 + data1_size, Config::MAX_BALANCING_TIME);
                        write<Stack>(data1, 0x0318);
                    }
                    if(Config::CELL_COUNT_PER_DEVICE > 8)
                    {
                        constexpr size_t data2_size = Config::CELL_COUNT_PER_DEVICE % 8;
                        uint8_t data2[data2_size] { };
                        std::fill(data2, data2 + data2_size, Config::MAX_BALANCING_TIME);
                        write<Stack>(data2, 0x0318 + 8);
                    }
                }
                /* "restart" balancing - when balance time is NOT 0 it will start and balance for the set time */
                {
                    BalCtrl2 bal_ctrl_2
                    {
                        .auto_bal = true,
                        .bal_go = true
                    };
                    // write<Stack>(0x03, address_of<BalCtrl2>());
                    write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                    write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                }

                return HAL_OK;
            }
        public:
            HAL_StatusTypeDef stop_balancing()
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;
                {
                    BalCtrl2 bal_ctrl_2
                    {
                        .auto_bal = false,
                        .bal_go = false,
                        .cb_pause = true
                    };
                    write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                    write<Stack>(convert_to<uint8_t>(bal_ctrl_2), address_of<BalCtrl2>());
                }

                return HAL_OK;
            }
        public:
            /**
             *  @brief eebbe
             *  @param voltage in mV
             *  @return ebebeb
             */
            HAL_StatusTypeDef set_balancing_voltage(uint32_t voltage)
            {
                using enum CommunicationMode;

                using namespace Utils;
                using namespace Regs;
                using namespace Types;

                /* set voltage */
                {
                    uint32_t vcb_done_thresh = std::clamp(2450ui32, voltage, 4000ui32);
                    uint8_t regval = (uint8_t)(((vcb_done_thresh - 2450) / 25 + 1) & 0x3f);

                    write<Stack>(regval, 0x032A);   
                }
                /* restart ovuv */
                {
                    uint8_t data = convert_to<uint8_t>((OVUVCtrl){ .ovuv_mode = ScanMode::RoundRobin, .ovuv_go = true });
                    
                    /* after init should re quire this only once */
                    write<Stack>(data, address_of<OVUVCtrl>());
                    write<Stack>(data, address_of<OVUVCtrl>());
                }

                return HAL_OK;
            }
        };
    }
}
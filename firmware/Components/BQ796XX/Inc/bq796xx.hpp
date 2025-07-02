#pragma once

#include "array"

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
            template<CommunicationMode COMMUNICATION_TYPE, size_t DATA_COUNT> requires(DATA_COUNT > 0 && DATA_COUNT <= 8)
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

                return uart->await_tx_dma(tx_buffer, i);
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

                return uart->await_tx_dma(tx_buffer, i);
            }
        private:
            template<CommunicationMode COMMUNICATION_TYPE, size_t DATA_COUNT, size_t DEVICE_COUNT>
            HAL_StatusTypeDef read(uint8_t (&data)[DEVICE_COUNT][DATA_COUNT], uint16_t reg_address, uint8_t dev_address = 1)
            {
                using enum CommunicationMode;

                size_t i = 0;
                uint8_t tx_buffer[7] { 0 };
                uint8_t rx_buffer[DEVICE_COUNT * DATA_COUNT + 6] { 0 };

                tx_buffer[i++] = 0b1'000'0000 | (uint8_t)(COMMUNICATION_TYPE);
                if constexpr(COMMUNICATION_TYPE == Single) tx_buffer[i++] = dev_address;
                tx_buffer[i++] = (uint8_t)(reg_address >> 8);
                tx_buffer[i++] = (uint8_t)(reg_address);
                tx_buffer[i++] = DATA_COUNT - 1;

                uint16_t crc = Crc16::fast(tx_buffer, i);
                tx_buffer[i++] = (uint8_t)(crc >> 8);
                tx_buffer[i++] = (uint8_t)(crc);

                // if(uart.await_tx_dma(buffer, i) != HAL_OK) return HAL_ERROR;
                // if(uart.await_rx_dma((uint8_t*)data, DEVICE_COUNT * DATA_COUNT + 2) != HAL_OK) return HAL_ERROR;

                return uart->await_tx_rx_dma(tx_buffer, i, rx_buffer, DEVICE_COUNT * DATA_COUNT + 2);
            }
        public:
            /**
             *  @brief 	Init Bq796xx, this function inits whole stack communication, voltage measurement and otut/ovuv protection
             *  @retval	HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
             */
            HAL_StatusTypeDef init()
            {
                using enum CommunicationMode;

                uart->set_rx_timeout(Config::RX_TIMEOUT_BAUDBLOCKS);
                
                if(wake_up() != HAL_OK) return HAL_ERROR;  
                
                tx_thread_sleep(4);

                /* cmd wake up slaves */
                write<Single>(Utils::convert_to<uint8_t>((Regs::Control1){.send_wake = true}), Utils::address_of<Regs::Control1>());

                /* wait ~15ms */
                tx_thread_sleep(15);

                /* dummy write 0x00, sync internal dlls */
                for(size_t step = 0; step < 8; step++)
                {
                    write<Broadcast>(0x00, 0x343 + step);
                }

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

                // TODO: internal status error
                /* verify */
                [[maybe_unused]] uint8_t buffer[Config::STACK_SIZE][1] { 0 };
                read<Stack>(buffer, 0x306);

                return HAL_OK;
            }
        };
    }
}
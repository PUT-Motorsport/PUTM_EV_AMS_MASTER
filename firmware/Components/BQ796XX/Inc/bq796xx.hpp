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
    namespace Bq796xx
    {
        class Device
        {
        private:
            Uart uart;
        public:
            /**
            *	@brief Init Bq796xx, set receiver timeout to t_rx_timeout (~300 us) for the uart handler to for data in IT or DMA mode
            *	@param `huart` uart handle
            */
            Device(UART_HandleTypeDef *huart) : uart(huart) { }
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
                if(uart.set_baudrate(Config::Bq796xx::Config::BAUDRATE_WAKEUP) != HAL_OK) return HAL_ERROR;
                if(uart.await_tx_dma(out, 1) != HAL_OK) return HAL_ERROR;
                if(uart.set_baudrate(Config::Bq796xx::Config::DEFAULT_BAUDRATE) != HAL_OK) return HAL_ERROR;

                return HAL_OK;
            }
        private:
            enum class CommunicationType : uint8_t
            {
                Single = 0b00000000,
                Stack = 0b00100000,
                Broadcast = 0b01000000
            };
            using enum CommunicationType;
        private:
            template<CommunicationType COMMUNICATION_TYPE, size_t DATA_COUNT> requires(DATA_COUNT > 0 && DATA_COUNT <= 8)
            HAL_StatusTypeDef write(uint8_t (&data)[DATA_COUNT], uint16_t reg_address, uint8_t dev_address = 1)
            {
                size_t i = 0;
                uint8_t tx_buffer[DATA_COUNT + 7] { 0 };

                tx_buffer[i++] = 0b1'001'0000 | (uint8_t)(COMMUNICATION_TYPE) | DATA_COUNT - 1;
                if constexpr(COMMUNICATION_TYPE == CommunicationType::Single)tx_buffer[i++] = dev_address;
                tx_buffer[i++] = (uint8_t)(reg_address >> 8);
                tx_buffer[i++] = (uint8_t)(reg_address);
                std::copy(data, data + DATA_COUNT, tx_buffer + i);
                i += DATA_COUNT;
                uint16_t crc = Crc16::fast(tx_buffer, i);
                data[i++] = (uint8_t)(crc >> 8);
                data[i++] = (uint8_t)(crc);

                return uart.await_tx_dma(data, i);
            }
        private:
            template<CommunicationType COMMUNICATION_TYPE, size_t DATA_COUNT, size_t DEVICE_COUNT>
            HAL_StatusTypeDef read(uint8_t (&data)[DEVICE_COUNT][DATA_COUNT], uint16_t reg_address, uint8_t dev_address = 1)
            {
                size_t i = 0;
                uint8_t tx_buffer[7] { 0 };

                tx_buffer[i++] = 0b1'000'0000 | (uint8_t)(COMMUNICATION_TYPE);
                if constexpr(COMMUNICATION_TYPE == CommunicationType::Single) buffer[i++] = dev_address;
                buffer[i++] = (uint8_t)(reg_address >> 8);
                buffer[i++] = (uint8_t)(reg_address);
                buffer[i++] = DEVICE_COUNT - 1;

                uint16_t crc = Crc16::fast(buffer, i);
                buffer[i++] = (uint8_t)(crc >> 8);
                buffer[i++] = (uint8_t)(crc);

                // if(uart.await_tx_dma(buffer, i) != HAL_OK) return HAL_ERROR;
                // if(uart.await_rx_dma((uint8_t*)data, DEVICE_COUNT * DATA_COUNT + 2) != HAL_OK) return HAL_ERROR;

                if(uart.await_tx_rx_dma(buffer, i, (uint8_t*)data, DEVICE_COUNT * DATA_COUNT + 2) != HAL_OK) return HAL_ERROR;

                return HAL_OK;
            }
        public:
            /**
             *  @brief 	Init Bq796xx, this function inits whole stack communication, voltage measurement and otut/ovuv protection
             *  @retval	HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
             */
            HAL_StatusTypeDef init()
            {
                uart.init();
                uart.set_rx_timeout(Config::Bq796xx::Config::RX_TIMEOUT_BAUDBLOX);
                
                if(wake_up() != HAL_OK) return HAL_ERROR;  
                
                tx_thread_sleep(4);

                data[0] = convert_to<uint8_t>((Control1){.send_wake = true});
                write<ReqType::Single>(data, 1, address_of<Control1>());

                {
                    uint8_t data[1] { 0 };
                }
            }
        }
    }
}
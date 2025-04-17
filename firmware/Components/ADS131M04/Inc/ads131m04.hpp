#pragma once

#include "array"

#include "tx_api.h"
#include "main.h"
#include "spi.h"

#include "utils.hpp"
#include "ads131m04/regs.hpp"
#include "ads131m04/cmd.hpp"

namespace PUTM
{
    namespace Ads131m04
    {
        class Device
        {
        private:
            SPI_HandleTypeDef *hspi;
            TX_SEMAPHORE semaphore;
            
            static inline constexpr size_t size = 6;

            /* V per 1 bit */
            static constexpr double v_lsb_adc = 2.4 / ( 16777216.0 * 256.0 );
    
            std::array<uint32_t, size> out { 0 };
            std::array<uint32_t, size> in { 0 };
    
        public:
            /* ADC data in [V]*/
            std::array<float, 4> adc { 0.f };
            Regs::Status status;
        public:
            explicit Device(SPI_HandleTypeDef *hspi);
        public:
            /*
             *  @brief  init semaphore
             */
            void init();
        public:
            /*
             *  @brief  update data
             *  @note   read function executes 2 times
             */
            void update();
        public:
            /*
             *  @brief  reset device
             *  @note   read function executes 2 times
             */   
            void reset();
        public:
            /*
             *  @brief 	update adc data
             *  @note 	function overrides spi callback
             *  @retval HAL status returns HAL_OK when new data data is read;
             */
            HAL_StatusTypeDef read(Cmd::ICmd *cmd);
        };
    }
}
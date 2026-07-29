#pragma once

#include "main.h"
#include "fdcan.h"

#include "array"

#ifdef DEBUG_PRINTF_ENABLE
#include "string.h"
#include "usart.h"
#endif /* DEBUG_PRINTF_ENABLE */

#include "data.hpp"

namespace PUTM
{
    class ChargerCanTxMessage
    {
    private:
        // TODO: ADD IT TO CONFIG
        // TODO: ADD can speed to config?
        constexpr static FDCAN_HandleTypeDef &hfdcan = hfdcan1;
        constexpr static uint32_t CHARGER_TX_ID = 0x1806E5F4;
        std::array<uint8_t, 8> __data;

        constexpr static FDCAN_TxHeaderTypeDef charger_transmit_header = []
        {
            FDCAN_TxHeaderTypeDef temp_header{};
            temp_header.Identifier = CHARGER_TX_ID;
            temp_header.IdType = FDCAN_EXTENDED_ID;
            temp_header.TxFrameType = FDCAN_DATA_FRAME;
            temp_header.DataLength = FDCAN_DLC_BYTES_8;
            temp_header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
            temp_header.BitRateSwitch = FDCAN_BRS_OFF;
            temp_header.FDFormat = FDCAN_CLASSIC_CAN;
            temp_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
            temp_header.MessageMarker = 0;
            return temp_header;
        }();

    public:
        constexpr ChargerCanTxMessage(float voltage, float current, bool enable)
        {
            __data = 
            {
                (uint8_t)((uint16_t)(voltage * 10) >> 8),
                (uint8_t)((uint16_t)(voltage * 10)),
                (uint8_t)((uint16_t)(current * 10) >> 8),
                (uint8_t)((uint16_t)(current * 10)),
                (uint8_t)(not enable),
                0,
                0,
                0
            };
        }

        HAL_StatusTypeDef send()
        {
            auto status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan,
                                                        const_cast<FDCAN_TxHeaderTypeDef *>(&charger_transmit_header),
                                                        const_cast<uint8_t *>(__data.begin()));
            return status;
        }
    };

    class ChargerCanRxController
    {
    private:
        constexpr static FDCAN_HandleTypeDef &hfdcan = hfdcan1;
        constexpr static uint32_t CHARGER_RX_ID = 0x18ff50e5;

        std::array<uint8_t, 8> __data { 0 };
        float battery_read_voltage { 0.0f };
        float battery_read_current { 0.0f };
        bool hardware_fail { false };
        bool over_temperature { false };
        bool in_voltage_fail { false };
        bool starting_state { false };
        bool communication_state { false };
        uint32_t last_recive_tick { 0 };
        bool charger_time_out { false };

        uint32_t canFifoMessageCount() const
        {
            return HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0);
        }

        void processFrame()
        {
            FDCAN_RxHeaderTypeDef header {};
            auto status = HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &header, __data.begin());

            if (CHARGER_RX_ID == header.Identifier and HAL_OK == status)
            {
                uint16_t voltage = ((uint16_t)(__data[0]) << 8) | ((uint16_t)__data[1]);
                uint16_t current = ((uint16_t)(__data[2]) << 8) | ((uint16_t)__data[3]);
                data.charger.battery_read_voltage = battery_read_voltage = voltage * 0.1f;
                data.charger.battery_read_current = battery_read_current = current * 0.1f;
                data.charger.hardware_fail = hardware_fail = __data[4] & 0x01;
                data.charger.over_temperature = over_temperature = __data[4] & 0x02;
                data.charger.in_voltage_fail = in_voltage_fail = __data[4] & 0x04;
                data.charger.starting_state = starting_state = __data[4] & 0x08;
                data.charger.communication_state = communication_state = __data[4] & 0x10;
                data.charger.last_recive_tick = last_recive_tick = tx_time_get();
            }
        }

    public:
        inline void update()
        {
            while (canFifoMessageCount() not_eq 0)
            {
                processFrame();
            }
        }

        constexpr float getBatteryVoltage() const
        {
            return battery_read_voltage;
        }

        constexpr float getBatteryCurrent() const
        {
            return battery_read_current;
        }
    };
}
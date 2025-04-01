#include "tx_api.h"

#include "ads131m04.hpp"
#include "ads131m04/regs.hpp"
#include "utils.hpp"
#include "wrapper/uart.hpp"

using namespace Utils;
using namespace PUTM;
using namespace PUTM::Ads131m04;
using namespace PUTM::Ads131m04::Regs;
using namespace PUTM::Ads131m04::Types;
using namespace PUTM::Ads131m04::Cmd;

Device::Device(SPI_HandleTypeDef *hspi) : hspi(hspi) { }

void Device::init()
{
    tx_semaphore_create(&semaphore, "semaphore", 0);
}

void Device::update()
{
    auto c = Cmd::CmdNull();
    read(&c);
    status = c.get_response();
}

void Device::reset()
{
    auto c = Cmd::CmdReset();
    read(&c);
}

HAL_StatusTypeDef Device::read(ICmd *cmd)
{
    volatile HAL_SPI_StateTypeDef state = hspi->State;

    if(state == HAL_SPI_STATE_RESET) Error_Handler();

    out.at(0) = cmd->cmd;
    std::copy(cmd->data + 0, cmd->data + 4, out.begin() + 1);

    pSPI_CallbackTypeDef callback = [](SPI_HandleTypeDef* hspi)
    {
        if(hspi->UserData == nullptr) Error_Handler();
        Device *ads = (Device*)hspi->UserData;

        tx_semaphore_put(&ads->semaphore);
    };

    hspi->UserData = (void*)this;
    hspi->TxRxCpltCallback = callback;

    if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)out.begin() , (uint8_t*)in.begin(), out.size()) != HAL_OK) Error_Handler();
    auto notify_received = tx_semaphore_get(&semaphore, 10);

    if(notify_received != 0) return HAL_ERROR;

    tx_thread_sleep(10);

    if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)out.begin() , (uint8_t*)in.begin(), out.size()) != HAL_OK) Error_Handler();
    notify_received = tx_semaphore_get(&semaphore, 10);
    hspi->UserData = nullptr;

    if(notify_received != 0) return HAL_ERROR;
    else
    {
        cmd->response = (uint16_t)(in[0] >> 8);

        for(size_t i = 1; i < 5; i++) in[i] <<= 8;
        adc[0] = (int32_t)in[1] * v_lsb_adc;
        adc[1] = (int32_t)in[2] * v_lsb_adc;
        adc[2] = (int32_t)in[3] * v_lsb_adc;
        adc[3] = (int32_t)in[4] * v_lsb_adc;
    }

    return HAL_OK;
}
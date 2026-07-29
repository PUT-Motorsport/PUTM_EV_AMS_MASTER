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

    {
        auto cmd = Cmd::CmdReset();
        com(&cmd);
    }
    {
        auto cmd = Cmd::CmdWReg<Regs::Cfg>();
        cmd.set_data((Regs::Cfg){ .gc_en = true, .gc_dly = GcDly::_16 });
        com(&cmd);
    }
    // {
    //     auto cmd = Cmd::CmdWReg<Regs::ThrshldLsb>();
    //     cmd.set_data((Regs::ThrshldLsb){ .dc_block = DcBlock::_1_4, .cd_th_lsb = 0x00 });
    //     com(&cmd);
    // }
    {
        auto cmd = Cmd::CmdWReg<Regs::Clock>();
        cmd.set_data((Regs::Clock){ .pwr = Pwr::HighRes, .osr = Osr::_1024, .tbm = false, .ch0_en = true, .ch1_en = true, .ch2_en = true, .ch3_en = true });
        com(&cmd);
    }
}

void Device::update()
{
    auto c = Cmd::CmdNull();
    com(&c);
    status = c.get_response();
}

void Device::reset()
{
    auto c1 = Cmd::CmdReset();
    com(&c1);

    auto c2 = Cmd::CmdWReg<Regs::Cfg>();
    c2.set_data((Regs::Cfg){ .gc_en = true, .gc_dly = GcDly::_16 });
    com(&c2);
}

void Device::enable_ch0_only()
{
    // auto c0 = Cmd::CmdWReg<Regs::ChCfg<0>>();
    // c0.set_data((Regs::ChCfg<0>){.mux = Mux::InPN});
    // com(&c0);

    // auto c1 = Cmd::CmdWReg<Regs::ChCfg<1>>();
    // c1.set_data((Regs::ChCfg<1>){.mux = Mux::AdcShorted});
    // com(&c1);

    // auto c2 = Cmd::CmdWReg<Regs::ChCfg<2>>();
    // c2.set_data((Regs::ChCfg<2>){.mux = Mux::AdcShorted});
    // com(&c2);

    // auto c3 = Cmd::CmdWReg<Regs::ChCfg<3>>();
    // c3.set_data((Regs::ChCfg<3>){.mux = Mux::AdcShorted});
    // com(&c3);

    // auto c = Cmd::CmdWReg<Regs::Cfg>();
    // c.set_data((Regs::Cfg){ .gc_en = true, .gc_dly = GcDly::_16 });
    // com(&c);
}

HAL_StatusTypeDef Device::com(ICmd *cmd)
{
    volatile HAL_SPI_StateTypeDef state = hspi->State;

    if(state == HAL_SPI_STATE_RESET) Error_Handler();

    out.at(0) = (uint32_t)cmd->cmd << 8;
    //std::copy(cmd->data + 0, cmd->data + 4, out.begin() + 1);
    for(size_t i = 0; i < 4; i++) 
    {
        out.at(i + 1) = (uint32_t)cmd->data[i] << 8;
    }

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

    //tx_thread_sleep(5);

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
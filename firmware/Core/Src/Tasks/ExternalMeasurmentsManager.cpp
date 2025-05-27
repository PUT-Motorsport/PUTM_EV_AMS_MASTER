/*
 * ExternalMeasurmentsManager.cpp
 *
 *  Created on: Jul 4, 2023
 *      Author: Piotr Lesicki
 */

#include <main.h>
#include <spi.h>
#include <numeric>
#include <algorithm>
#include <functional>
#include <optional>

#include <app_freertos.h>
#include <Controllers/Mcp356xController.hpp>
#include <StackData.hpp>

static SPI_HandleTypeDef &hspi = hspi3;

static Mcp356xController car(GpioOut(NMES_CAR_CS_GPIO_Port, NMES_CAR_CS_Pin, true), hspi);
static Mcp356xController acu(GpioOut(NMES_ACU_CS_GPIO_Port, NMES_ACU_CS_Pin, true), hspi);
static Mcp356xController isens(GpioOut(NMES_ISENS_CS_GPIO_Port, NMES_ISENS_CS_Pin, true), hspi);

// num of cyckles after which the adc mes will be restarted in case no new data was received
static constexpr uint32_t timeout = 4;

enum struct ISensMeasurement
{
	Current,
	Reference
};

static constexpr Mcp356x::ConfigGroup std_config = []()
{
	Mcp356x::ConfigGroup c;
	c.adc_mode = Mcp356x::AdcMode::StandBy;
	c.bias_current = Mcp356x::BiasCurrent::_0uA;
	c.clk_sel = Mcp356x::ClockSelect::InternalNoOutput;
	c.shut_down = Mcp356x::ShutDown::Active;
	c.oversampling_ratio = Mcp356x::OversamplingRatio::_16384;
	c.aclk_prescaller_div = Mcp356x::AClkPrescallerDiv::_4;
	// AutoZeroMux will perform Chx - Chy & Chy - Chx (minimizes mes offset)
	c.az_mux = Mcp356x::AutoZeroMux::Enabled;
	c.gain = Mcp356x::Gain::_1;
	c.boost = Mcp356x::Boost::_2;
	c.en_gain_cal = false;
	c.en_off_cal = false;
	c.en_crc = false;
	c.crc_format = Mcp356x::CrcFormat::_16bit;
	c.data_format = Mcp356x::DataFormat::_24bit_right_sgn;
	c.conv_mode = Mcp356x::ConvMode::Continuous;
	c.en_stop = true;
	c.en_fastcmd = true;
	// IrqPinState must be HighState
	c.irq_pin_state = Mcp356x::IrqPinState::HighState;
	c.irq_mdat_selection = Mcp356x::IrqMdatSelection::All;
	return c;
}();

static constexpr Mcp356x::ConfigGroup isens_config = []()
{
	Mcp356x::ConfigGroup c;
	c.adc_mode = Mcp356x::AdcMode::StandBy;
	c.bias_current = Mcp356x::BiasCurrent::_15uA;
	c.clk_sel = Mcp356x::ClockSelect::InternalNoOutput;
	c.shut_down = Mcp356x::ShutDown::Active;
	c.oversampling_ratio = Mcp356x::OversamplingRatio::_16384;
	c.aclk_prescaller_div = Mcp356x::AClkPrescallerDiv::_4;
	// AutoZeroMux will perform Chx - Chy & Chy - Chx (minimizes mes offset)
	c.az_mux = Mcp356x::AutoZeroMux::Enabled;
	c.gain = Mcp356x::Gain::_0_33;
	c.boost = Mcp356x::Boost::_2;
	c.en_gain_cal = false;
	c.en_off_cal = false;
	c.en_crc = false;
	c.crc_format = Mcp356x::CrcFormat::_16bit;
	c.data_format = Mcp356x::DataFormat::_24bit_right_sgn;
	c.conv_mode = Mcp356x::ConvMode::Continuous;
	c.en_stop = true;
	c.en_fastcmd = true;
	// IrqPinState must be HighState
	c.irq_pin_state = Mcp356x::IrqPinState::HighState;
	c.irq_mdat_selection = Mcp356x::IrqMdatSelection::All;
	return c;
}();

extern GpioIn charger_detect;
extern GpioIn safety_detect;
extern GpioIn air_m_detect;
extern GpioIn air_p_detect;
extern GpioIn air_pre_detect;

static float bcr = 0.f;

float currentReggression(float u)
{
	constexpr float a = 171.5f;
	//constexpr float b = -1.16f;
	return a * u + bcr;
}

constexpr float voltageReggression(float u)
{
	constexpr float a = 282.8;
	constexpr float b = -0.47f;
	return a * u + b;
}

void vExternalMeasurmentsManagerTask(void *argument)
{
	car.init();
	acu.init();
	isens.init();

	car.configure(std_config);
	acu.configure(std_config);
	isens.configure(isens_config);

	car.setChannels(Mcp356x::MuxIn::Ch0, Mcp356x::MuxIn::Ch1);
	acu.setChannels(Mcp356x::MuxIn::Ch0, Mcp356x::MuxIn::Ch1);
	isens.setChannels(Mcp356x::MuxIn::Ch0, Mcp356x::MuxIn::Ch1);

	car.restartAdc();
	acu.restartAdc();
	isens.restartAdc();

	while(true)
	{
		osDelay(10);

		FullStackDataInstance::set().external.charger_connected = charger_detect.isActive();
		FullStackDataInstance::set().external.tsms_on 			= safety_detect.isActive();

		FullStackDataInstance::set().external.m_state 	= air_m_detect.isActive();
		FullStackDataInstance::set().external.p_state 	= air_p_detect.isActive();
		FullStackDataInstance::set().external.pre_state = air_pre_detect.isActive();

		if(car.dataReady())
		{
			float u = voltageReggression(car.readVoltage());
			FullStackDataInstance::set().external.car_volt = u;
		}
		if(acu.dataReady())
		{
			float u = voltageReggression(acu.readVoltage());
			FullStackDataInstance::set().external.acu_volt = u;
		}
		if(isens.dataReady())
		{
			static ISensMeasurement isens_meas { ISensMeasurement::Current };
			switch (isens_meas)
			{
				case ISensMeasurement::Current:
				{
					float i = currentReggression(isens.readVoltage());
					FullStackDataInstance::set().external.acu_curr = i;
					static bool done { false };
					if(HAL_GetTick() > 2000 and not done)
					{
						bcr = -i;
						done = true;
					}
					isens.setChannels(Mcp356x::MuxIn::Ch1, Mcp356x::MuxIn::Agnd);
					isens.restartAdc();
					isens_meas = ISensMeasurement::Reference;
				} break;
				case ISensMeasurement::Reference:
				{
					float ref = isens.readVoltage();
					FullStackDataInstance::set().external.isens_ref = ref;
					isens.setChannels(Mcp356x::MuxIn::Ch0, Mcp356x::MuxIn::Ch1);
					isens.restartAdc();
					isens_meas = ISensMeasurement::Current;
				} break;
			}
		}
	}
}



/*
 * testTask.cpp
 *
 *  Created on: Jan 17, 2025
 *      Author: lenovo
 */

#include "app_freertos.h"
#include "main.h"
#include "gpio_wrap.hpp"
#include "tim_wrap.hpp"
#include "array"
#include "tim.h"
#include "ads131m04.hpp"
#include "bq796xx.hpp"

Gpio led_err(LED_ERROR_GPIO_Port, LED_ERROR_Pin, true);
Gpio led_wrn(LED_WARNING_GPIO_Port, LED_WARNING_Pin, true);
Gpio led_ok(LED_OK_GPIO_Port, LED_OK_Pin, true);
Gpio sig_err(SIG_AMS_ERROR_GPIO_Port, SIG_AMS_ERROR_Pin, true);
Gpio en_12v(EN_12V_GPIO_Port, EN_12V_Pin, false);
Gpio sig_air_pre(SIG_AIR_PRE_GPIO_Port, SIG_AIR_PRE_Pin, false);
Gpio sig_air_p(SIG_AIR_P_GPIO_Port, SIG_AIR_P_Pin, false);
Gpio sig_air_m(SIG_AIR_M_GPIO_Port, SIG_AIR_M_Pin, false);
Gpio det_air_pre(DET_AIR_PRE_GPIO_Port, DET_AIR_PRE_Pin, false);
Gpio det_air_p(DET_AIR_P_GPIO_Port, DET_AIR_P_Pin, false);
Gpio det_air_m(DET_AIR_M_GPIO_Port, DET_AIR_M_Pin, false);
Gpio det_tsms(DET_TSMS_GPIO_Port, DET_TSMS_Pin, false);
Gpio det_charger(DET_CHARGER_GPIO_Port, DET_CHARGER_Pin, false);
Gpio adc_dry(ADC_NDRY_GPIO_Port, ADC_NDRY_Pin, true);

Tim fan1(&htim2, TIM_CHANNEL_1);
Tim fan2(&htim2, TIM_CHANNEL_2);
Tim fan3(&htim2, TIM_CHANNEL_3);
Tim fan4(&htim2, TIM_CHANNEL_4);
Tim fan5(&htim3, TIM_CHANNEL_1);
Tim fan6(&htim3, TIM_CHANNEL_2);
Tim fan7(&htim3, TIM_CHANNEL_3);

Adc::Ads131m04 adc(&hspi1);

std::array<Tim*, 7> fans { &fan1, &fan2, &fan3, &fan4, &fan5, &fan6, &fan7 };
std::array<Gpio*, 5> gpios_ins { &det_air_pre, &det_air_p, &det_air_m, &det_tsms, &det_charger };
std::array<bool, 5> gpios_state;

void startTestTask([[maybe_unused]] void *argument)
{
	adc.init();

	sig_err.reset();

	[[maybe_unused]] float i = 0.f;
	[[maybe_unused]] float iii = 25.f;
	[[maybe_unused]] float inc = iii;

	[[maybe_unused]] size_t k = 1;
	[[maybe_unused]] int kinc = -1;

	//en_12v.set();
	en_12v.reset();

	for(auto& f : fans) f->startPwm();

	led_ok.set();

	while(true)
	{
		osDelay(500);

		adc.update();

//		led_err.toggle();
//		led_wrn.toggle();
//		led_ok.toggle();
//		sig_air_pre.toggle();
//		sig_air_p.toggle();
//		sig_air_m.toggle();
//
//		sig_air_m.set(det_air_m.read());
//		sig_air_p.set(det_air_p.read());
//		sig_air_pre.set(det_air_pre.read());
//
//		if(i >= 100.f) inc = -iii;
//		if(i <= 0.f) inc = iii;
//
//		if(i <= 0)
//		{
//			if(k == 6) kinc = -1;
//			if(k == 0) kinc = 1;
//			k += kinc;
//		}
//
//		i += inc;
//
//		fans[k]->setFill(i / 10.f);
//
//		for(size_t i = 0; i < 5; i++)
//		{
//			gpios_state[i] = gpios_ins[i]->read();
//		}
	}
}

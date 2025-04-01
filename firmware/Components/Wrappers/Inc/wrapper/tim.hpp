/*
 * tim_wrap.hpp
 *
 *  Created on: Feb 8, 2025
 *      Author: Piotr lesicki
 */

#ifndef INC_TIM_WRAP_HPP_
#define INC_TIM_WRAP_HPP_

#include "main.h"
#include "algorithm"

class Tim
{
private:
	uint32_t channel;

	TIM_HandleTypeDef *htim;

public:
	/*
	 *	@brief set modulation as a value from 0 to TIM::Period
	 *	@param htim TIM_HandleTypeDef timer handle
	 *	@param channel uint32_t TIM_CHANNEL_X
	 */
	explicit Tim(TIM_HandleTypeDef *htim, uint32_t channel) : channel(channel), htim(htim) { };

	/*
	 *	@brief start pwm
	 */
	void startPwm()
	{
		HAL_TIM_PWM_Start(htim, channel);
	}

	/*
	 *	@brief set modulation as a value from 0 to TIM::Period
	 *	@param comp uint32_t value from 0 to TIM::Perdiod
	 */
	void setCompare(uint32_t comp)
	{
		auto period = htim->Init.Period + 1;
		comp = std::min(period, comp);
		__HAL_TIM_SET_COMPARE(htim, channel, comp);
	}

	/*
	 *	@brief set modulation as a percentage of a full range
	 *	@param x float 0.f to 100.f
	 */
	void setFill(float x)
	{
		x = std::clamp(x, 0.f, 100.f);
		float period = float(htim->Init.Period + 1);
		uint32_t comp = uint32_t(period * x / 100.f);
		__HAL_TIM_SET_COMPARE(htim, channel, comp);
	}
};

#endif /* INC_TIM_WRAP_HPP_ */

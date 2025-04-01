/*
 * gpio_wrap.hpp
 *
 *  Created on: Jan 17, 2025
 *      Author: Piotr Lesicki
 */

#ifndef INC_GPIO_WRAP_HPP_
#define INC_GPIO_WRAP_HPP_

#include "main.h"

class Gpio
{
public:
	explicit Gpio(GPIO_TypeDef *gpio, uint32_t pin, bool inv) : gpio(gpio), pin(pin), inv(uint32_t(inv)) { };

	void set()
	{
		HAL_GPIO_WritePin(gpio, pin, GPIO_PinState(inv ^ uint32_t(GPIO_PIN_SET)));
	}

	void set(bool state)
	{
		HAL_GPIO_WritePin(gpio, pin, GPIO_PinState(inv ^ uint32_t(state)));
	}

	void reset()
	{
		HAL_GPIO_WritePin(gpio, pin, GPIO_PinState(inv ^ uint32_t(GPIO_PIN_RESET)));
	}

	void toggle()
	{
		HAL_GPIO_TogglePin(gpio, pin);
	}

	bool read()
	{
		return bool(uint32_t(HAL_GPIO_ReadPin(gpio, pin)) ^ inv);
	}

private:
	GPIO_TypeDef *gpio;
	uint32_t pin;
	uint32_t inv;
};


#endif /* INC_GPIO_WRAP_HPP_ */

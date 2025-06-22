/*
 * Config.hpp
 *
 *  Created on: 3 sie 2023
 *      Author: Piotr Lesicki
 */

#ifndef INC_INTERFACES_CONFIG_HPP_
#define INC_INTERFACES_CONFIG_HPP_

#include <main.h>
#include <array>
#include <typeinfo>
#include <type_traits>

#include <Interfaces/MCP356xRegs.hpp>

namespace LtcConfig
{
	static constexpr size_t CHAIN_SIZE = 12;

//	static constexpr float UNDERVOLTAGE = 3.02f;
//	static constexpr float OVERVOLTAGE = 4.18f;

	static constexpr float OPEN_WIRE_MIN_VOLTAGE = 2.f;
	static constexpr float OPEN_WIRE_MAX_VOLTAGE = 5.f;

	//max 12;
	static constexpr size_t CELLS_PER_LTC = 10;
	//needs to be the size of CELLS_PER_LTC (max 12) and be indexed from 0-11
	static constexpr std::array < size_t, CELLS_PER_LTC > CELL_TO_CH_MAP { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10 };
	//static constexpr std::array < size_t, CELLS_PER_LTC > CELL_TO_CH_MAP { 10, 9, 8, 7, 6, 4, 3, 2, 1, 0 };
	static constexpr size_t CELL_COUNT = CHAIN_SIZE * CELLS_PER_LTC;

	//max 5
	static constexpr size_t TEMP_PER_LTC = 4;
	//needs to be the size of TEMP_PER_LTC (max 5) and be indexed from 0-4
	static constexpr std::array < size_t, TEMP_PER_LTC > TEMP_TO_CH_MAP { 0, 1, 3, 4 };
	static constexpr size_t TEMP_COUNT = CHAIN_SIZE * TEMP_PER_LTC;
}

namespace ChecksConfig
{
	constexpr static float CELL_MAX_VOLTAGE = 4.18f;
	constexpr static float CELL_MIN_VOLTAGE = 3.02f;
	constexpr static int32_t VOLTAGE_ERROR_COUNT_MAX = 40;
	constexpr static float IMBALANCE_MAX_VOLTAGE = 0.3f;
	constexpr static float CELL_MAX_TEMPERATURE = 55.0f;
	constexpr static float CELL_MIN_TEMPERATURE = -20.0f;
	constexpr static int32_t TEMP_ERROR_COUNT_MAX = 20;
	constexpr static float BATTERY_MAX_CURRENT = 150.0f;
	constexpr static float BATTERY_SENSOR_DISCONNECT = 350.0f;
	constexpr static int32_t CURRENT_ERROR_COUNT_MAX = 40;
}

namespace Mcp356xConfig
{
	constexpr static float ADC_REF_P = 3.0;
	constexpr static float ADC_REF_M = 0.0;
	constexpr static float ADC_REF = ADC_REF_P - ADC_REF_M;
}

namespace ExternalConfig
{
	constexpr static float ADC_VOLT_REF = 3.0;
	constexpr static float ADC_VOLT_RES = 2'000.0;
	constexpr static float ADC_VOLT_COEF = (800'000.0 + ADC_VOLT_RES) / ADC_VOLT_RES;
	constexpr static float ADC_CURR_COEF = 200.0 / 1.25;
}

namespace PlausibilityConfig
{
	constexpr static uint32_t min_precharge_time = 2000;  	//
	constexpr static float precharge_min_charge = 0.95f;		// in 0.0 - 1.0
}

namespace ChargerConfig
{
	constexpr static uint32_t balance_time = 5000;
}

namespace LaGimela
{
	//ltc, meas - 0 - 13, 0 - 3
	constexpr static std::array<std::pair<size_t, size_t>, 3> pomijanie_temperatury = { { { 11, 2 }, { 3, 2 }, { 12, 3 } } };
	constexpr static std::array<std::pair<size_t, size_t>, 1> naprawianie_napiec = {{{ 1, 0 }}};
}

namespace
{
	// DO NOT TOUCH, DONT U DARE
	namespace AssertLtcCellConfig
	{
		// DO NOT TOUCH, DONT U DARE
		namespace CheckCellMap
		{
			template < size_t i >
			static constexpr bool subCheck()
			{
				return LtcConfig::CELL_TO_CH_MAP[i] >= 0 && LtcConfig::CELL_TO_CH_MAP[i] <= 11;
			}

			template < size_t i >
			static constexpr bool check()
			{
				if constexpr(i == 0) return subCheck<0>();
				else return subCheck<i>() && check<i - 1>();
			}

			static constexpr bool checkCellToChMap()
			{
				return check<LtcConfig::CELLS_PER_LTC - 1>();
			}
		}

		// DO NOT TOUCH, DONT U DARE
		static_assert(LtcConfig::CELL_TO_CH_MAP.size() <= 12);
		static_assert(LtcConfig::CELL_TO_CH_MAP.size() >= 1);

		static_assert(LtcConfig::CELLS_PER_LTC <= 12);
		static_assert(LtcConfig::CELLS_PER_LTC >= 1);

		static_assert(LtcConfig::CELL_TO_CH_MAP.size() == LtcConfig::CELLS_PER_LTC);
//		idk shit bellow doesnt work
//		static_assert(std::is_same_v<decltype(LtcConfig::CELL_TO_CH_MAP), std::array<size_t, LtcConfig::CELLS_PER_LTC>>);

		static_assert(CheckCellMap::checkCellToChMap(), "\"LtcConfig::CELL_TO_CH_MAP\" check failed: element > 11 or < 0");
	};

	// DO NOT TOUCH, DONT U DARE
	namespace AssertLtcTempConfig
	{
		// DO NOT TOUCH, DONT U DARE
//		namespace CheckTempMap
//		{
//			template < size_t i >
//			static constexpr bool subCheck()
//			{
//				return LtcConfig::TEMP_TO_CH_MAP[i] >= 0 && LtcConfig::TEMP_TO_CH_MAP[i] <= 4;
//			}
//
//			template < size_t i >
//			static constexpr bool check()
//			{
//				if constexpr(i == 0) return subCheck<0>();
//				else return subCheck<i>() && check<i - 1>();
//			}
//
//			static constexpr bool checkCellToChMap()
//			{
//				return check<LtcConfig::TEMP_PER_LTC - 1>();
//			}
//		}

		static_assert(LtcConfig::TEMP_PER_LTC <= 5);
		static_assert(LtcConfig::TEMP_PER_LTC >= 1);
	};

}

#endif /* INC_INTERFACES_CONFIG_HPP_ */

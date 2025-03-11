/*
 * ads131m04.hpp
 *
 *  Created on: Feb 12, 2025
 *      Author: Piotr Lesicki
 */

#ifndef INC_ADS131M04_HPP_
#define INC_ADS131M04_HPP_

#include "main.h"
#include "concepts"
#include "spi.h"
#include "array"
#include "functional"
#include "atomic"
#include "cstring"

using afloat32_t = std::atomic<float_t>;
using auint32_t = std::atomic<uint32_t>;

static_assert(USE_HAL_SPI_REGISTER_CALLBACKS == 1UL, "Use registered callback for SPI");
static_assert(offsetof(SPI_HandleTypeDef, UserData), "Add \"void* userData;\" to SPI_HandleTypeDef implementation");

namespace Ads131m04
{
	namespace Regs
	{
		struct IReg { };

		struct Id : public IReg
		{
		private:
			const uint16_t reserved0: 8  { 0b000000000 };
		public:
			const uint16_t chancnt: 4  { 0b0000 };
		private:
			const uint16_t reserved1: 4  { 0b0000 };
		};

		enum struct CrcType : uint16_t
		{
			CCITT,
			ANSI
		};

		struct Status : public IReg
		{
		public:
			const bool drdy0: 1 { 0b0 };
			const bool drdy1: 1 { 0b0 };
			const bool drdy2: 1 { 0b0 };
			const bool drdy3: 1 { 0b0 };
		private:
			const uint16_t reserved0: 4  { 0b0000 };
		public:
			const uint16_t wlength: 2 { 0b0 };
			const bool reset: 1 { 0b0 };
			const CrcType crc_type: 1 { 0b0 };
			const bool crc_err: 1 { 0b0 };
			const bool reg_map: 1 { 0b0 };
			const bool f_resync: 1 { 0b0 };
			const bool lock: 1 { 0b0 };
		};

		struct Mode : public IReg
		{
		public:
			bool drdy_fmt: 1;
			bool drdy_hiz: 1;
			uint16_t drdy_sel: 2;
			bool timeout: 1;
		private:
			const uint16_t reserved0: 3 = { 0b000 };
		public:
			uint16_t wlength: 2;
			bool reset: 1;
			CrcType crc_type: 1;
			bool rx_crc_en: 1;
			bool reg_crc_en: 1;
		private:
			const uint16_t reserved1: 2 = { 0b00 };
		};

		enum struct Pwr : uint16_t
		{
			VeryLowPower,
			LowPower,
			HighRes,
			HighRes2
		};

		enum struct Osr
		{
			_128,
			_256,
			_512,
			_1024,
			_2048,
			_4096,
			_8192,
			_16256
		};

		struct Clock : public IReg
		{
		public:
			Pwr pwr: 2;
			Osr osr: 3;
			bool tbm: 1;
		private:
			const uint16_t reserved0: 2 = { 0b00 };
		public:
			bool ch0_en: 1;
			bool ch1_en: 1;
			bool ch2_en: 1;
			bool ch3_en: 1;
		private:
			const uint16_t reserved1: 4 = { 0b0000 };
		};

		enum struct PgaGain
		{
			_1,
			_2,
			_4,
			_8,
			_16,
			_32,
			_64,
			_128,
		};

		struct Gain : public IReg
		{
		public:
			PgaGain pga_gain0 : 3;
		private:
			const uint16_t reserved0: 1 { 0b0 };
		public:
			PgaGain pga_gain1 : 3;
		private:
			const uint16_t reserved1: 1 { 0b0 };
		public:
			PgaGain pga_gain2 : 3;
		private:
			const uint16_t reserved2: 1 { 0b0 };
		public:
			PgaGain pga_gain3 : 3;
		private:
			const uint16_t reserved3: 1 { 0b0 };
		};

		struct Reserved
		{
		private:
			const uint16_t reserved0 { 0x0000 };
		};

		enum struct CdLen : uint16_t
		{
			_128,
			_256,
			_512,
			_768,
			_1280,
			_1792,
			_2560,
			_3584
		};

		enum struct CdNum : uint16_t
		{
			_1,
			_2,
			_4,
			_8,
			_16,
			_32,
			_64,
			_128,
		};

		enum struct GcDly : uint16_t
		{
			_2,
			_4,
			_8,
			_16,
			_32,
			_64,
			_128,
			_256,
			_512,
			_1024,
			_2048,
			_4096,
			_8192,
			_16384,
			_32768,
			_65536
		};

		struct Cfg : public IReg
		{
		public:
			bool cd_en: 1;
			CdLen cd_len: 3;
			CdNum cd_num : 3;
			bool cd_allch: 1;
			bool gc_en: 1;
			GcDly gc_dly: 4;
		private:
			const uint16_t reserved0: 3 { 0b000 };
		};

		struct ThrshldMsb : public IReg
		{
		public:
			uint16_t cd_th_msb;
		};

		enum struct DcBlock : uint16_t
		{
			dis,
			_1_4,
			_1_8,
			_1_16,
			_1_32,
			_1_64,
			_1_128,
			_1_256,
			_1_512,
			_1_1024,
			_1_2048,
			_1_4096,
			_1_8192,
			_1_16384,
			_1_32768,
			_1_65536
		};

		struct ThrshldLsb : public IReg
		{
		public:
			DcBlock dc_block : 4;
		private:
			const uint16_t reserved0 : 4 { 0b0000 };
		public:
			uint16_t cd_th_lsb: 8;
		};

		enum struct Mux : uint16_t
		{
			InPM,
			AdcShorted,
			DcTestP,
			DcTestN
		};

		template<int T>
		concept IsChannel = T <= 3 and T >= 0;

		// for channels 0-3
		template<int Ch> requires IsChannel<Ch>
		struct ChCfg : public IReg
		{
		public:
			Mux mux: 2;
			bool dcblk_dis: 1;
		private:
			const uint16_t reserved: 3 { 0b000 };
		public:
			uint16_t phase: 10;
		};

		// for channels 0-3

		template<int Ch> requires IsChannel<Ch>
		struct ChOcalMsb : public IReg
		{
		public:
			uint16_t ocal_msb;
		};

		// for channels 0-3

		template<int Ch> requires IsChannel<Ch>
		struct ChOcalLsb : public IReg
		{
		public:
			uint16_t ocal_lsb: 8;
		private:
			const uint16_t reserved0: 8;
		};

		// for channels 0-3

		template<int Ch> requires IsChannel<Ch>
		struct ChGcalMsb : public IReg
		{
		public:
			uint16_t gcal_msb;
		};

		// for channels 0-3

		template<int Ch> requires IsChannel<Ch>
		struct ChGcalLsb : public IReg
		{
		public:
			uint16_t gcal_lsb: 8;
		private:
			const uint16_t reserved0: 8;
		};

		struct RegMapCrc : public IReg
		{
		public:
			uint16_t reg_crc;
		};
	}

	namespace Utils
	{
		template<typename T>
		concept IsReg = std::is_base_of<Regs::IReg, T>::value and not std::is_polymorphic<T>::value and sizeof(T) == 2;

		void throw_consteval_failure(char const*);

		/*
		 * @brief DO NOT USE for insiders only B), changes struct type into register address on compile time
		 * @ret uint8_t register address corresponding to register struct
		 */
		template<typename T> requires IsReg<T>
		static uint8_t constexpr sta()
		{
			if constexpr(std::same_as<T, Regs::Id>) 			return 0x00;
			if constexpr(std::same_as<T, Regs::Status>)			return 0x01;
			if constexpr(std::same_as<T, Regs::Mode>)			return 0x02;
			if constexpr(std::same_as<T, Regs::Clock>)			return 0x03;
			if constexpr(std::same_as<T, Regs::Gain>)			return 0x04; //yes 0x05 is missing
			if constexpr(std::same_as<T, Regs::Cfg>)			return 0x06;
			if constexpr(std::same_as<T, Regs::ThrshldMsb>)		return 0x07;
			if constexpr(std::same_as<T, Regs::ThrshldLsb>)		return 0x08;

			if constexpr(std::same_as<T, Regs::ChCfg<0>>)		return 0x09;
			if constexpr(std::same_as<T, Regs::ChOcalMsb<0>>)	return 0x0a;
			if constexpr(std::same_as<T, Regs::ChOcalLsb<0>>)	return 0x0b;
			if constexpr(std::same_as<T, Regs::ChGcalMsb<0>>)	return 0x0c;
			if constexpr(std::same_as<T, Regs::ChGcalLsb<0>>)	return 0x0d;

			if constexpr(std::same_as<T, Regs::ChCfg<1>>)		return 0x0e;
			if constexpr(std::same_as<T, Regs::ChOcalMsb<1>>)	return 0x0f;
			if constexpr(std::same_as<T, Regs::ChOcalLsb<1>>)	return 0x10;
			if constexpr(std::same_as<T, Regs::ChGcalMsb<1>>)	return 0x11;
			if constexpr(std::same_as<T, Regs::ChGcalLsb<1>>)	return 0x12;

			if constexpr(std::same_as<T, Regs::ChCfg<2>>)		return 0x13;
			if constexpr(std::same_as<T, Regs::ChOcalMsb<2>>)	return 0x14;
			if constexpr(std::same_as<T, Regs::ChOcalLsb<2>>)	return 0x15;
			if constexpr(std::same_as<T, Regs::ChGcalMsb<2>>)	return 0x16;
			if constexpr(std::same_as<T, Regs::ChGcalLsb<2>>)	return 0x17;

			if constexpr(std::same_as<T, Regs::ChCfg<3>>)		return 0x18;
			if constexpr(std::same_as<T, Regs::ChOcalMsb<3>>)	return 0x19;
			if constexpr(std::same_as<T, Regs::ChOcalLsb<3>>)	return 0x1a;
			if constexpr(std::same_as<T, Regs::ChGcalMsb<3>>)	return 0x1b;
			if constexpr(std::same_as<T, Regs::ChGcalLsb<3>>)	return 0x1c;

			if constexpr(std::same_as<T, Regs::RegMapCrc>)		return 0x3e;

			throw_consteval_failure("failed to evaluate sta");
			return 0xff;
		}
	}

	namespace Cmd
	{
		constexpr uint16_t Null = 		0b0000'0000'0000'0000;
		constexpr uint16_t Reset = 		0b0000'0000'0001'0001;
		constexpr uint16_t StandBy =	0b0000'0000'0010'0010;
		constexpr uint16_t WakeUp =		0b0000'0000'0011'0011;
		constexpr uint16_t Lock =		0b0000'0101'0101'0101;
		constexpr uint16_t UnLock =		0b0000'0110'0101'0101;

		/*
		 * @brief ReadReg cmd constructor
		 * @ret corresponding read command for the required address register
		 */
		constexpr uint16_t read_reg(uint16_t addr, uint16_t count = 0)
		{
			// cmd = 101a aaaa accc cccc;
			uint16_t a = addr & 0b11'1111 << 7;
			uint16_t c = count & 0b111'1111;
			uint16_t cmd = 0b1010'0000'0000'0000 | a | c;
			return cmd;
		}
		/*
		 * @brief WriteReg cmd constructor
		 * @ret corresponding write command for the required address register
		 */
		constexpr uint16_t write_reg(uint16_t addr, uint16_t count = 0)
		{
			// cmd = 101a aaaa accc cccc;
			uint16_t a = addr & 0b11'1111 << 7;
			uint16_t c = count & 0b111'1111;
			uint16_t cmd = 0b0110'0000'0000'0000 | a | c;
			return cmd ;
		}
	}

	/*
	 * @brief set spi in 24bit mode and enable register callbacks
	 */
	class Ads131m04
	{
	private:
		template<typename T> requires Utils::IsReg<T>
		uint32_t construct_write()
		{
			constexpr uint32_t addr = Utils::sta<T>();
			//constexpr uint16_t count = 1; //serial write not supported
			constexpr uint32_t cmd = Cmd::write_reg(addr);

			return cmd;
		}

		template<typename T> requires Utils::IsReg<T>
		uint32_t construct_read()
		{
			constexpr uint16_t addr = Utils::sta<T>();
			//constexpr uint16_t count = 1; //serial write not supported
			constexpr uint32_t cmd = Cmd::read_reg(addr);

			return cmd;
		}

		SPI_HandleTypeDef *hspi;

		bool newData { false };

		static inline constexpr size_t size = 6;

		std::array<uint32_t, size> out { 0 };
		std::array<uint32_t, size> in { 0 };

	public:
		std::array<float, 4> adc { 0.f };
		Regs::Status status;

		explicit Ads131m04(SPI_HandleTypeDef *hspi) : hspi(hspi) { }

		/*
		 * @brief none for now
		 */
		void init()
		{
			return;
		}

		/*
		 * @brief 	update adc data
		 * @note 	function overrides spi callback while in use then returns them to their default state
		 * 			wait is realized with HAL_Delay override it if you are using any rtos
		 * @retval 	HAL status returns HAL_OK when new data is avalable otherwise returns busy
		 */
		HAL_StatusTypeDef update()
		{
			if(hspi->State == HAL_SPI_STATE_RESET) Error_Handler();
			if(hspi->State == HAL_SPI_STATE_ABORT or hspi->State == HAL_SPI_STATE_ERROR) Error_Handler();

			if(hspi->State == HAL_SPI_STATE_READY and newData) return HAL_OK;

			if(hspi->State != HAL_SPI_STATE_READY) return HAL_BUSY;


			out[0] = construct_read<Regs::Status>();
			
			hspi->UserData = (void*)this;

			pSPI_CallbackTypeDef callback = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Ads131m04 *adc = (Ads131m04*)hspi->UserData;
				uint32_t *in = (uint32_t*)adc->in.begin();

				if(hspi->State != HAL_SPI_STATE_ERROR and hspi->State != HAL_SPI_STATE_ABORT)
				{
					uint32_t tmp = in[0] >> 8;
					std::memcpy((uint8_t*)&adc->status, (uint8_t*)&tmp, sizeof(uint16_t));

					const float coef = 1.2f / 16777215.f;
					adc->adc[0] = in[1] * coef;
					adc->adc[1] = in[2] * coef;
					adc->adc[2] = in[3] * coef;
					adc->adc[3] = in[4] * coef;
				}

				if(HAL_SPI_UnRegisterCallback(hspi, HAL_SPI_TX_RX_COMPLETE_CB_ID) != HAL_OK) Error_Handler();

				adc->newData = true;
			};

			//use inherence to go around this shit
			if(HAL_SPI_RegisterCallback(hspi, HAL_SPI_TX_RX_COMPLETE_CB_ID, callback) != HAL_OK) Error_Handler();

			if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)out.begin() , (uint8_t*)in.begin(), out.size()) != HAL_OK) Error_Handler();

			newData = false;

			return HAL_BUSY;
		}
	};
}

#endif /* INC_ADS131M04_HPP_ */

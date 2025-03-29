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
#include "FreeRTOS.h"

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
			bool drdy0: 1 { 0b0 };
			bool drdy1: 1 { 0b0 };
			bool drdy2: 1 { 0b0 };
			bool drdy3: 1 { 0b0 };
			uint16_t reserved0: 4  { 0b0000 };
			uint16_t wlength: 2 { 0b0 };
			bool reset: 1 { 0b0 };
			CrcType crc_type: 1 { 0b0 };
			bool crc_err: 1 { 0b0 };
			bool reg_map: 1 { 0b0 };
			bool f_resync: 1 { 0b0 };
			bool lock: 1 { 0b0 };
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
		consteval uint16_t read_reg(uint16_t addr, uint16_t count = 0)
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
		consteval uint16_t write_reg(uint16_t addr, uint16_t count = 0)
		{
			// cmd = 101a aaaa accc cccc;
			uint16_t a = addr & 0b11'1111 << 7;
			uint16_t c = count & 0b111'1111;
			uint16_t cmd = 0b0110'0000'0000'0000 | a | c;
			return cmd ;
		}

		struct Cmd
		{
			uint16_t cmd;
			uint32_t data[4];
			uint16_t crc;
			uint16_t response;
		};

		struct CmdNull : public Cmd
		{
			CmdNull()
			{
				cmd = 0x00;
				std::fill(data + 0, data + 4, 0x00);
				crc = 0x00;
			}

			Regs::Status get_response()
			{
				return *((Regs::Status*)&response);
			}
		};

		struct CmdReset : public Cmd
		{
			CmdReset()
			{
				cmd = 0x0101;
				std::fill(data + 0, data + 4, 0x00);
				crc = 0x00;
			}
		};

		template<typename T>
		struct CmdRReg : public Cmd
		{
			CmdRReg()
			{
				cmd = read_reg(Utils::sta<T>());
				std::fill(data + 0, data + 4, 0x00);
				crc = 0x00;
			}

			T get_response()
			{
				return *((T*)&response);
			}
		};
	}

	/*
	 * @brief set spi in 24bit mode and enable register callbacks
	 */
	class Ads131m04
	{
	private:
		SPI_HandleTypeDef *hspi;

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

		void update()
		{
			auto c = Cmd::CmdNull();
			cmd(&c);
			status = c.get_response();
		}

		void reset()
		{
			auto c = Cmd::CmdReset();
			cmd(&c);
		}

		/*
		 * @brief none for now
		 */

		private:
			struct CmdHandle
			{
				TaskHandle_t task;
			} hcmd;
		public:
		/*
		 * @brief 	update adc data
		 * @note 	function overrides spi callback
		 * @retval 	HAL status returns HAL_OK when new data data is read;
		 */
		HAL_StatusTypeDef cmd(Cmd::Cmd *cmd)
		{
			volatile HAL_SPI_StateTypeDef state = hspi->State;

			if(state == HAL_SPI_STATE_RESET) Error_Handler();

			out.at(0) = cmd->cmd;
			std::copy(cmd->data + 0, cmd->data + 4, out.begin() + 1);

			pSPI_CallbackTypeDef callback = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Ads131m04 *ads = (Ads131m04*)hspi->UserData;
				int32_t *in = (int32_t*)ads->in.begin();

				vTaskNotifyGiveFromISR(ads->hcmd.task, nullptr);
			};

			hcmd.task = xTaskGetCurrentTaskHandle();
			hspi->UserData = (void*)this;
			hspi->TxRxCpltCallback = callback;

			if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)out.begin() , (uint8_t*)in.begin(), out.size()) != HAL_OK) Error_Handler();
			auto notify_received = xTaskNotifyWait(0x00, 0xffffffff, nullptr, 10);
			
			if(notify_received != pdTRUE) return HAL_ERROR;

			osDelay(10);

			if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)out.begin() , (uint8_t*)in.begin(), out.size()) != HAL_OK) Error_Handler();
			notify_received = xTaskNotifyWait(0x00, 0xffffffff, nullptr, 10);
			hspi->TxRxCpltCallback = HAL_SPI_TxRxCpltCallback;

			if(notify_received != pdTRUE) return HAL_ERROR;
			else
			{
				cmd->response = (uint16_t)(in[0] >> 8);

				static constexpr double v_lsb_adc = 2.4 / ( 16777216.0 * 256.0 );

				for(size_t i = 1; i < 5; i++) in[i] <<= 8;
				adc[0] = (int32_t)in[1] * v_lsb_adc;
				adc[1] = (int32_t)in[2] * v_lsb_adc;
				adc[2] = (int32_t)in[3] * v_lsb_adc;
				adc[3] = (int32_t)in[4] * v_lsb_adc;
			}

			return HAL_OK;
		}
	};
}

#endif /* INC_ADS131M04_HPP_ */

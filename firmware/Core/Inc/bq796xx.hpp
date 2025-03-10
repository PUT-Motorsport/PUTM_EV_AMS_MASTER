/*
 * bq79616.hpp
 *
 *  Created on: Feb 18, 2025
 *      Author: lenovo
 */

#ifndef INC_BQ796XX_HPP_
#define INC_BQ796XX_HPP_

#include "main.h"
#include "concepts"
#include "utility"
#include "spi.h"
#include "gpio.h"
#include "cmath"
#include "crc16ibm.hpp"

//============================================================================================//
// make sure its the input clock on spi not the spi clock after prescaler
static constexpr double SPI_CLOCK = 6e6; 
//============================================================================================//

namespace Bq
{
	namespace Utils
	{
		struct IReg { };

		template<size_t START, size_t OFFSET = 0>
		struct IAddress
		{
		    static inline constexpr int ADDRESS = START + OFFSET;
		};

		template<typename T>
		concept IsReg = std::is_base_of<IReg, T>::value and not std::is_polymorphic<T>::value and sizeof(T) == 1;

		template<typename T>
		int constexpr sta() { return T::ADDRESS; }

		template<typename T>
		uint8_t tob(T s)
		{
			return (uint8_t)*(void*)&s;
		}

		void throw_consteval_failure(char const*);

		struct Dir0AddrOtp : public IReg, IAddress<0x0000>
		{
			uint8_t undef { 0x00 };
		};

		struct __packed Dir1AddrOtp : public IReg, IAddress<0x0001>
		{
			uint8_t undef { 0x00 };
		};

		template<size_t T>
		concept IsMiscReg = T >= 1 and T <= 9;

		template<size_t MiscReg> requires IsMiscReg<MiscReg>
		struct __packed CustMisc : public IReg
		{
			uint8_t data;
		};

		struct __packed Dir0Addr : public IReg, IAddress<0x0306>
		{
			uint8_t undef;
		};

		struct __packed Dir1Addr : public IReg, IAddress<0x0307>
		{
			uint8_t undef;
		};

		struct __packed PartId : public IReg, IAddress<0x0500>
		{
			uint8_t part_id;
		};

		struct __packed DevRevId : public IReg, IAddress<0x0e00>
		{
			uint8_t undef;
		};

		template<size_t T>
		concept IsDieIdReg = T >= 1 and T <= 9;

		template<size_t DieIdReg> requires IsDieIdReg<DieIdReg>
		struct __packed DieId : public IReg
		{
			uint8_t id;
		};
		//undef

		struct __packed DevConf : public IReg, IAddress<0x0002>
		{
		public:
			uint8_t hb_en : 1 { 0b0 };
			uint8_t ftone_en : 1 { 0b0 };
			uint8_t nfault_en : 1 { 0b1 };
			uint8_t two_stop_en : 1 { 0b0 };
			uint8_t fcomm_en : 1 { 0b1 };
			uint8_t multidrop_en : 1 { 0b0 };
			uint8_t no_adj_cb : 1 { 0b1 };
		private:
			uint8_t reserved : 1 { 0b0 };
		};

		enum struct __packed NumCell : uint8_t
		{
			_6S,
			_7S,
			_8S,
			_9S,
			_10S,
			_11S,
			_12S,
			_13S,
			_14S,
			_15S,
			_16S
		};

		struct __packed ActiveCell : public IReg, IAddress<0x0003>
		{
		public:
			NumCell num_cell : 4 { NumCell::_16S };
		private:
			uint8_t spare : 4 { 0b0000 };
		};

		struct __packed BbvcPosn1 : public IReg
		{
			uint8_t cell9  : 1 { 0b0 };
			uint8_t cell10 : 1 { 0b0 };
			uint8_t cell11 : 1 { 0b0 };
			uint8_t cell12 : 1 { 0b0 };
			uint8_t cell13 : 1 { 0b0 };
			uint8_t cell14 : 1 { 0b0 };
			uint8_t cell15 : 1 { 0b0 };
			uint8_t cell16 : 1 { 0b0 };
		};
		//undef

		struct __packed BbvcPosn2 : public IReg
		{
			uint8_t cell1 : 1 { 0b0 };
			uint8_t cell2 : 1 { 0b0 };
			uint8_t cell3 : 1 { 0b0 };
			uint8_t cell4 : 1 { 0b0 };
			uint8_t cell5 : 1 { 0b0 };
			uint8_t cell6 : 1 { 0b0 };
			uint8_t cell7 : 1 { 0b0 };
			uint8_t cell9 : 1 { 0b0 };
		};
		//undef

		enum struct __packed SleepTime : uint8_t
		{
			No,
			_5s,
			_10s,
			_1min,
			_10min,
			_30min,
			_1h,
			_2h
		};

		enum struct __packed TWarnThr : uint8_t
		{
			_85C,
			_95C,
			_105C,
			_115C
		};

		struct __packed PwrTransitConf : public IReg, IAddress<0x0018>
		{
		public:
			SleepTime slp_time : 3 { 0b000 };
			TWarnThr twarn_thr : 2 { 0b11 };
		private:
			uint8_t spare : 3 { 0b000 };
		};

		enum struct __packed CtsTime : uint8_t
		{
			No,
			_100ms,
			_2s,
			_10s,
			_10min,
			_30min,
			_1h
		};

		enum struct __packed CtlTime : uint8_t
		{
			No,
			_100ms,
			_2s,
			_10s,
			_1min,
			_10min,
			_30min,
			_1h
		};

		struct __packed CommTimeoutConf : public IReg, IAddress<0x0019>
		{
		public:
			CtlTime ctl_time : 3 { 0b000 };
			bool ctl_act : 1 { 0b0 };
			CtsTime cts_time : 3 { 0b000 };
		private:
			uint8_t spare : 1 { 0b0 };
		};

		struct __packed TxHoldOff : public IReg, IAddress<0x001a>
		{
			uint8_t dly { 0x00 };
		};

		struct __packed StackResponse : public IReg, IAddress<0x0029>
		{
		public:
			uint8_t dly : 6 { 0b000000 };
		private:
			uint8_t spare : 2 { 0b00 };
		};

		enum struct __packed Loc : uint8_t
		{
			No,
			Undef,
		};

		struct __packed BBPLock : public IReg
		{
		public:
			Loc lock : 5 { 0b00000 };
		private:
			uint8_t spare : 3 { 0b000 };
		};
		//undef

		struct __packed CommCtrl : public IReg, IAddress<0x0308>
		{
		public:
			bool top_stack : 1 { 0b0 };
			bool stack_dev : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};

		struct __packed Control1 : public IReg, IAddress<0x0309>
		{
			bool addr_wr : 1 { 0b0 };
			bool soft_reset : 1 { 0b0 };
			bool goto_sleep : 1 { 0b0 };
			bool goto_shutdown : 1 { 0b0 };
			bool send_slptoact : 1 { 0b0 };
			bool send_wake : 1 { 0b0 };
			bool send_shutdown : 1 { 0b0 };
			bool dir_sel : 1 { 0b0 };
		};

		struct __packed Control2 : public IReg, IAddress<0x030a>
		{
		public:
			bool tsref_en : 1 { 0b0 };
			bool send_hw_reset : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};

		enum struct __packed Byte : uint16_t
		{
			Hi,
			Lo
		};

		template<Byte T>
		concept IsByte = T == Byte::Hi or T == Byte::Lo;

		template<Byte B> requires IsByte<B>
		struct __packed CustCrc: public IReg
		{
			uint8_t undef { 0x00 };
		};

		template<Byte B> requires IsByte<B>
		struct __packed CustCrcRslt : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed DiagStat : public IReg, IAddress<0x0526>
		{
		public:
			bool drdy_bist_pwr : 1 { 0b0 };
			bool drdy_bist_ovuv : 1 { 0b0 };
			bool drdy_bist_otut : 1 { 0b0 };
			bool drdy_ovuv : 1 { 0b0 };
			bool drdy_otut : 1 { 0b0 };
		private:
			uint8_t reserved : 3 { 0b000 };
		};

		struct __packed AdcStat1 : public IReg, IAddress<0x0527>
		{
		public:
			bool drdy_main_adc : 1 { 0b0 };
			bool drdy_aux_misc : 1 { 0b0 };
			bool drdy_aux_cell : 1 { 0b0 };
			bool drdy_aux_gpio : 1 { 0b0 };
		private:
			uint8_t reserved : 4 { 0b0 };
		};

		struct __packed AdcStat2 : public IReg, IAddress<0x0528>
		{
		public:
			bool drdy_vccb : 1 { 0b0 };
			bool drdy_cbfet : 1 { 0b0 };
			bool drdy_cbow : 1 { 0b0 };
			bool drdy_vcow : 1 { 0b0 };
			bool drdy_gpio : 1 { 0b0 };
			bool drdy_lpf : 1 { 0b0 };
		private:
			uint8_t reserved : 2 { 0b00 };
		};

		struct __packed GpioStat : public IReg, IAddress<0x052a>
		{
			bool gpio1 : 1 { 0b0 };
			bool gpio2 : 1 { 0b0 };
			bool gpio3 : 1 { 0b0 };
			bool gpio4 : 1 { 0b0 };
			bool gpio5 : 1 { 0b0 };
			bool gpio6 : 1 { 0b0 };
			bool gpio7 : 1 { 0b0 };
			bool gpio8 : 1 { 0b0 };
		};

		struct __packed BalStat : public IReg, IAddress<0x052b>
		{
			bool cb_done : 1 { 0b0 };
			bool mb_done : 1 { 0b0 };
			bool abortflt : 1 { 0b0 };
			bool cb_run : 1 { 0b0 };
			bool mb_run : 1 { 0b0 };
			bool cb_inpause : 1 { 0b0 };
			bool ot_pause_det : 1 { 0b0 };
			bool invalid_cbconf : 1 { 0b0 };
		};

		struct __packed DevStat : public IReg, IAddress<0x052c>
		{
		public:
			bool main_run : 1 { 0b0 };
			bool aux_run : 1 { 0b0 };
		private:
			bool reserved1 : 1 { 0b0 };
		public:
			bool ovuv_run : 1 { 0b0 };
			bool otut_run : 1 { 0b0 };
			bool cust_crc_done : 1 { 0b0 };
			bool fact_crc_done : 1 { 0b0 };
		private:
			bool reserved2 : 1 { 0b0 };
		};

//		enum struct __packed SettleTime
//		{
//			_4_3ms,
//			_2_3ms,
//			_1_3ms,
//			Reserved
//		};
//
//		enum struct __packed CutOff
//		{
//			_6_5Hz,
//			_13Hz,
//			_26Hz,
//			_53Hz,
//			_111
//		};

		struct __packed AdcConf1 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed AdcConf2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed MainAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed MainAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed AuxAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct __packed AuxAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		enum struct __packed MainMode : uint8_t
		{
			No,
			Single,
			Continous
		};

		struct __packed AdcCtrl1 : public IReg, IAddress<0x030d>
		{
		public:
			MainMode main_mode : 2 { 0b00 };
			bool main_go : 1 { 0b0 };
			bool lpf_cell_en : 1 { 0b0 };
			bool lpf_bb_en : 1 { 0b0 };
		private:
			uint8_t reserved : 3 { 0b000 };
		};

		// those are kept at zero so whatever
		struct __packed AdcCtrl2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		// those are kept at zero so whatever
		struct __packed AdcCtrl3 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		template<size_t T>
		concept IsCellChannel = T >= 1 and T <= 16;

		template<Byte BYTE, size_t CHANNEL> requires IsCellChannel<CHANNEL> and IsByte<BYTE>
		struct __packed VCell : public IReg, IAddress<0x0568, size_t(BYTE) + CHANNEL>
		{
			uint8_t result;
		};


		template<Byte B> requires IsByte<B>
		struct __packed BusBar : public IReg
		{
			uint8_t result;
		};
		//undef

		template<Byte BYTE> requires IsByte<BYTE>
		struct __packed TsRef : public IReg, IAddress<0x058C, size_t(BYTE)>
		{
			uint8_t result;
		};

		template<size_t T>
		concept IsGpioChannel = T >= 1 and T <= 8;

		template<Byte BYTE, size_t CHANNEL> requires IsGpioChannel<CHANNEL> and IsByte<BYTE>
		struct __packed Gpio : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed DieTemp1 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed DieTemp2 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxCell : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxGpio : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxBat : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxRefl : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxVbg2 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxAvaoRef : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxAvddRef : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxOVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxUVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxOTCBDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxUTDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed AuxVCBdoneDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed RefOVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed DiagMain : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct __packed DiagHi : public IReg
		{
			uint8_t result;
		};

		template<size_t CellChannel> requires IsCellChannel<CellChannel>
		struct __packed CBCellCtrl : public IReg
		{
			uint8_t undef;
		};

		struct __packed VMBDoneThresh : public IReg
		{
			uint8_t undef;
		};

		struct __packed MBTimerCtrl : public IReg
		{
			uint8_t undef;
		};

		struct __packed CBDoneThresh : public IReg
		{
			uint8_t undef;
		};

		struct __packed OTCBThresh : public IReg
		{
			uint8_t undef;
		};

		struct __packed BalCtrl1 : public IReg
		{
			uint8_t undef;
		};

		struct __packed BalCtrl2 : public IReg
		{
			uint8_t undef;
		};

		struct __packed BalCtrl3 : public IReg
		{
			uint8_t undef;
		};

		struct __packed CBComplete1 : public IReg
		{
			bool cell9_done : 1 { 0b0 };
			bool cell10_done : 1 { 0b0 };
			bool cell11_done : 1 { 0b0 };
			bool cell12_done : 1 { 0b0 };
			bool cell13_done : 1 { 0b0 };
			bool cell14_done : 1 { 0b0 };
			bool cell15_done : 1 { 0b0 };
			bool cell16_done : 1 { 0b0 };
		};

		struct __packed CBComplete2 : public IReg
		{
			bool cell1_done : 1 { 0b0 };
			bool cell2_done : 1 { 0b0 };
			bool cell3_done : 1 { 0b0 };
			bool cell4_done : 1 { 0b0 };
			bool cell5_done : 1 { 0b0 };
			bool cell6_done : 1 { 0b0 };
			bool cell7_done : 1 { 0b0 };
			bool cell8_done : 1 { 0b0 };
		};

		struct __packed BalTime : public IReg
		{
			uint8_t undef;
		};

		struct __packed OVThresh : public IReg
		{
			uint8_t undef;
		};

		enum struct ReqType : uint8_t
		{
			Single 		= 0,
			Stack		= 1,
			Broadcast	= 2
		};

		template<ReqType REQ_TYPE> 
		concept IsReqType = 0 <= (uint8_t)REQ_TYPE and (uint8_t)REQ_TYPE <= 2;

		template<ReqType REQ_TYPE> requires IsReqType<REQ_TYPE>
		consteval uint8_t req_type_write()
		{
			switch(REQ_TYPE)
			{
			case ReqType::Single:
				return 0b001;
			case ReqType::Stack:
				return 0b011;
			case ReqType::Broadcast:
				return 0b101;
			}

			throw_consteval_failure("WRONG");
		}

		template<ReqType REQ_TYPE> requires IsReqType<REQ_TYPE>
		consteval size_t req_data_to_read(size_t chain_size)
		{
			switch(REQ_TYPE)
			{
			case ReqType::Single:
				return 1;
			case ReqType::Stack:
				return chain_size;
			case ReqType::Broadcast:
				return chain_size + 1;
			}

			throw_consteval_failure("WRONG");
		}

		template<ReqType REQ_TYPE> requires IsReqType<REQ_TYPE>
		consteval uint8_t req_type_read()
		{
			switch(REQ_TYPE)
			{
			case ReqType::Single:
				return 0b000;
			case ReqType::Stack:
				return 0b010;
			case ReqType::Broadcast:
				return 0b100;
			}
			throw_consteval_failure("WRONG");
		}

		struct __packed Init
		{
		public:
			Init(uint8_t req_type, uint8_t data_size) : data_size(data_size - 1), req_type(req_type) { }
			uint8_t data_size : 3 { 0 };
		private:
			const uint8_t rsvd : 1 { 0 };
		public:
			uint8_t req_type : 3 { 0 };
		private:
			uint8_t frame_type : 1 { 1 };
		};

	}

	template<size_t CHAIN_SIZE> requires ( CHAIN_SIZE <= 64 )
	class Bq796xx
	{
	public:
		Bq796xx(UART_HandleTypeDef *huart) : huart(huart) { }
	private:
		UART_HandleTypeDef huart;

		/*
		* 	@brief 	This funciton evalueates how much space is needed for buffers
		* 	@retval	evaluated min size needed
		*/
		// consteval size_t EVAL_NEEDED_BITS()
		// {
		// 	constexpr double CLK_PERIOD = 1 / SPI_CLOCK * 256;
		// 	constexpr size_t NEEDED_BITS = std::round(0.0025 / CLK_PERIOD);

		// 	static_assert(NEEDED_BITS <= 256, "too much needed bits");
		// 	static_assert(NEEDED_BITS * CLK_PERIOD > 0.003, "SPI is to slow sadge");
		// 	static_assert(NEEDED_BITS * CLK_PERIOD < 0.0025, "SPI is to fast sadge");

		// 	return NEEDED_BITS;
		// }

		/*
		* 	@brief 	This funciton evalueates how much space is needed for buffers
		* 	@retval	evaluated min size needed
		*/
		// consteval size_t EVAL_SIZE()
		// {
		// 	// up to 16 bytes + 2 init + 2 addr + 2 crc
		// 	constexpr size_t REQUIRED_BY_CHAIN = CHAIN_SIZE * (22);
			
		// 	if constexpr(EVAL_NEEDED_BITS() > REQUIRED_BY_CHAIN) return EVAL_NEEDED_BITS();

		// 	return REQUIRED_BY_CHAIN;
		// }

		std::array<uint8_t, 256> out { 0 };
		std::array<uint8_t, 256> in { 0 };

	public:
		/*
		* 	@brief 	This function inits all bq in a stach
		* 	@retval	HAL_BUSY when init is in progress is in progress, HAL_OK when done
		*/
		HAL_StatusTypeDef init()
		{
			static size_t state = 0;
		}

	private:
		bool wake_up_done { false };
		uint32_t prev_baud_rate_prescale { 0 };

		pUART_CallbackTypeDef callback_read { nullptr };
		pUART_CallbackTypeDef callback_write { nullptr };
	public:
		/*
		* 	@brief 	Wake up function for BQ79600 IC, this functions tries to hold the MOSI line
		*			for aprox ~2.5ms
		* 	@retval	HAL_BUSY when wakeing up is in progress, HAL_OK when done
		*/
		HAL_StatusTypeDef wake_up()
		{
			// prevent override during checks
			volatile uint32_t state = huart->State;

			if(state == HAL_UART_STATE_RESET) Error_Handler();
			if(state == HAL_SPI_STATE_ABORT or state == HAL_SPI_STATE_ERROR) Error_Handler();

			if(state == HAL_SPI_STATE_READY and wake_up_done) { wake_up_done = false; return HAL_OK; }

			if(state != HAL_SPI_STATE_READY) return HAL_BUSY;

			wake_up_done = false;

			if(HAL_SPI_DeInit(hspi) != HAL_OK) Error_Handler();
			prev_baud_rate_prescale = hspi->Init.BaudRatePrescaler;
			hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
			if(HAL_SPI_Init(hspi) != HAL_OK) Error_Handler();

			std::fill(out.begin(), out.begin() + EVAL_NEEDED_BITS(), 0);

			hspi->UserData = (void*)this;

			// FIXME: might not work
			callback_write = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)hspi->UserData;

				if(hspi->State == HAL_SPI_STATE_ERROR or hspi->State == HAL_SPI_STATE_ABORT) Error_Handler();

				if(HAL_SPI_DeInit(hspi) != HAL_OK) Error_Handler();
				bq->hspi->Init.BaudRatePrescaler = bq->prev_baud_rate_prescale;
				if(HAL_SPI_Init(hspi) != HAL_OK) Error_Handler();

				hspi->TxCpltCallback = nullptr;

				bq->wake_up_done = true;
			};

			hspi->TxCpltCallback = callback_write;

			if(HAL_SPI_Transmit_DMA(hspi, (uint8_t*)out.begin(), EVAL_NEEDED_BITS())!= HAL_OK) Error_Handler();

			return HAL_BUSY;
		}

	private:
		bool wite_done { false };
	public:
		/*
		* 	@brief 	Send `data` of `size` to a device at `address`. If data was send before this function
		*			can be called without any parameters to check the `writeSingle` state: `HAL_BUSY` (sending)
		*			or `HAL_OK` (done)
		* 	@tparam	`REG` first register to write, if more registers are writen they need to have an incrementing address
		*	@tparam	`REQ_TYPE` write type
		* 	@param 	`data` data, cant be largen than 8 bytes
		*	@param 	`address` address of a device to be written to - assumes 0
		* 	@retval	HAL_BUSY when writeSingle is in progress, HAL_OK when done or caller provided no data/size
		*/
		template<typename REG, Utils::ReqType REQ_TYPE, uint8_t SIZE> requires (SIZE <= 8 and Utils::IsReg<REG> and Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef write(std::array<uint8_t, SIZE> &data, uint8_t address = 0)
		{
			using namespace Utils;

			// prevent override during checks
			volatile uint32_t state = hspi->State;

			if(state == HAL_SPI_STATE_RESET) Error_Handler();
			if(state == HAL_SPI_STATE_ABORT or state == HAL_SPI_STATE_ERROR) Error_Handler();

			if(state == HAL_SPI_STATE_READY and wite_done) { wite_done = false; return HAL_OK; } 

			if(state != HAL_SPI_STATE_READY) return HAL_BUSY;

			hspi->UserData = (void*)this;
			
			out.at(0) = tob(Init(1, req_type_write<REQ_TYPE>(), data.size()));

			if(address > 0x3f) address = 0x3f;
			out.at(1) = address;

			uint16_t reg_addr = sta<REG>();
			out.at(2) = (uint8_t)reg_addr >> 8;
			out.at(3) = (uint8_t)reg_addr;

			uint16_t crc = fastcrc16ibm(out.begin(), 4);
			out.at(4) = (uint8_t)(crc >> 8);
			out.at(5) = (uint8_t)(crc);

			callback_write = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)hspi->UserData;

				if(hspi->State == HAL_SPI_STATE_ERROR or hspi->State == HAL_SPI_STATE_ABORT) Error_Handler();

				hspi->TxCpltCallback = HAL_SPI_TxCpltCallback;
				
				hspi->UserData = nullptr;
				bq->write_done = true;
			};

			hspi->TxCpltCallback = callback_write;

			if(HAL_SPI_Transmit_DMA(hspi, (uint8_t*)out.begin(), data.size() + 6) != HAL_OK) Error_Handler();

			return HAL_BUSY;
		}

	private:
		bool read_done { false };
		size_t data_counter { 0 };
	public:
		/*
		* 	@brief 	Send `data` of `size` to a device at `address`. If data was send before this function
		*			can be called without any parameters to check the `writeSingle` state: `HAL_BUSY` (sending)
		*			or `HAL_OK` (done)
		* 	@tparam	`REG` first register to write, if more registers are writen they need to have an incrementing address
		*	@tparam	`REQ_TYPE` write type
		* 	@param 	`data` data, cant be largen than 8 bytes
		*	@param 	`address` address of a device to be written to - assumes 0
		* 	@retval	HAL_BUSY when writeSingle is in progress, HAL_OK when done or caller provided no data/size
		*/
		template<typename REG, Utils::ReqType REQ_TYPE, uint8_t SIZE> requires (1 <= SIZE and SIZE <= 8 and Utils::IsReg<REG> and Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef read(std::array<std::array<uint8_t, SIZE>, CHAIN_SIZE> &data, uint8_t address = 0)
		{
			using namespace Utils;

			// prevent override during checks
			volatile uint32_t state = hspi->State;

			if(state == HAL_SPI_STATE_RESET) Error_Handler();
			if(state == HAL_SPI_STATE_ABORT or state == HAL_SPI_STATE_ERROR) Error_Handler();

			if(state == HAL_SPI_STATE_READY and read_done) { read_done = false; return HAL_OK; } 

			if(state != HAL_SPI_STATE_READY) return HAL_BUSY;

			hspi->UserData = (void*)this;
			hgpio->UserData = (void*)this;
			data_counter = req_data_to_read<REQ_TYPE>(CHAIN_SIZE);
			
			out.at(0) = tob(Init(1, req_type_read<REQ_TYPE>(), data.size()));

			if(address > 0x3f) address = 0x3f; 
			out.at(1) = address;

			uint16_t reg_addr = sta<REG>();
			out.at(2) = (uint8_t)reg_addr >> 8;
			out.at(3) = (uint8_t)reg_addr;

			std::copy(data.begin(), data.end(), out.begin() + 4);
			
			uint16_t crc = fastcrc16ibm(out.begin(), data.size() + 4);
			out.at(data.size() + 4) = (uint8_t)(crc >> 8);
			out.at(data.size() + 5) = (uint8_t)(crc);

			callback_read = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)hspi->UserData;

				if(hspi->State == HAL_SPI_STATE_ERROR or hspi->State == HAL_SPI_STATE_ABORT) Error_Handler();

				if(bq->data_counter == 0) { hspi->RxCpltCallback = HAL_SPI_RxCpltCallback; bq->read_done = true; }
			};

			callback_write = [](SPI_HandleTypeDef* hspi)
			{
				if(hspi->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)hspi->UserData;

				if(hspi->State == HAL_SPI_STATE_ERROR or hspi->State == HAL_SPI_STATE_ABORT) Error_Handler();

				hspi->TxCpltCallback = HAL_SPI_TxCpltCallback;
				//if(HAL_SPI_RegisterCallback(hspi, HAL_SPI_RX_COMPLETE_CB_ID, callback_read) != HAL_OK) Error_Handler();
			};
			
			callback_exit = [](GPIO_HandleTypeDef* hgpio)
			{
				if(hgpio->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)hgpio->UserData;

				if(HAL_SPI_Receive_DMA(bq->hspi, (uint8_t*)bq->out.begin(), 6) != HAL_OK) Error_Handler();
			};

			hspi->TxCpltCallback = callback_write;
			hspi->RxCpltCallback = callback_read;
			hgpio->Exit1RisingCallback = callback_exit;

			if(HAL_SPI_Transmit_DMA(hspi, (uint8_t*)out.begin(), 6) != HAL_OK) Error_Handler();

			return HAL_BUSY;
		}
	};
}

#endif /* INC_BQ796XX_HPP_ */

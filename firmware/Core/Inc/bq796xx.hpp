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
#include "uart_wrap.hpp"
#include "crc16ibm.hpp"

namespace Bq796xx
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
		uint16_t constexpr sta() { return T::ADDRESS; }

		template<typename T>
		uint8_t tob(T s)
		{
			return (uint8_t)*(uint8_t*)&s;
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
		uint8_t init_byte_write(uint8_t data_size)
		{
			// uint8_t rsvd = 0b0000'0000;
			uint8_t frame_type = 0b1'0000000;
			uint8_t req_type = 0b0'001'0000 | (uint8_t)REQ_TYPE << 5;
			if(data_size > 8) data_size = 8;

			return frame_type | req_type | data_size;
		}

		template<ReqType REQ_TYPE> requires IsReqType<REQ_TYPE>
		uint8_t init_byte_read()
		{
			// uint8_t rsvd = 0b0000'0000;
			uint8_t frame_type = 0b1'0000000;
			uint8_t req_type = 0b0'000'0000 | (uint8_t)REQ_TYPE << 5;\
			// uint8_t data_size = 0b0000'0000;

			return frame_type | req_type;
		}
	}

	template<size_t CHAIN_SIZE> requires ( CHAIN_SIZE <= 64 )
	class Bq796xx
	{
	private: 
		/* Bq79600 has a preset baud rate of 1Mbps [bps] */
		constexpr static inline uint32_t default_baudrate = 1'000'000;
		/* t hold wakeup, in range <2500, 3000> [us] */
		constexpr static inline uint32_t t_wakeup = 2'750;
		/* rx timeout [us] (2 * defualt) */
		constexpr static inline uint32_t t_rx_timeout = 500;
		/* baudrate for init [bps], assume 1 bit high before, 6 bits of low time and 1 bit high after, this should create the required pattern */
		constexpr static inline uint32_t baudrate_wakeup = (uint32_t)(1'000'000.0 / (double)t_wakeup * 6.0);
		
		/*
		* 	
		*/
		template<Utils::ReqType REQ_TYPE> requires Utils::IsReqType<REQ_TYPE>
		static consteval size_t read_count()
		{
			switch (REQ_TYPE)
			{
			case Utils::ReqType::Single:
				return 1;
			case Utils::ReqType::Stack:
				return CHAIN_SIZE;
			case Utils::ReqType::Broadcast:
				return CHAIN_SIZE + 1;
			}
			Utils::throw_consteval_failure("WRONG");
		}
		
		/*
		*	@brief 	convert us and baudrate to needed baudblocks
		*	@param `time` in us
		*	@param `baudrate` in bps
		*	@return	baudblocks
		*/
		static consteval uint32_t to_bb(uint32_t time, uint32_t baudrate) { return (uint32_t)((double)(1.0 / time) * (double)baudrate); }

	public:
		/*
		*	@brief init Bq796xx, set receiver timeout to t_rx_timeout (~300 us) for the uart handler to for data in IT or DMA mode
		*	@param `huart` uart handle
		*/
		Bq796xx(UART_HandleTypeDef *huart) : huart(huart) { }
	private:
		UART_HandleTypeDef *huart;

		pUART_CallbackTypeDef callback_read { nullptr };
		pUART_CallbackTypeDef callback_write { nullptr };

		/* for now leave the size at 256 */
		std::array<uint8_t, 256> out { 0 };
		/* for now leave the size at 256 */
		std::array<uint8_t, 256> in { 0 };

		size_t init_state { 0 };
		size_t dummy_write_step { 0 };
		size_t auto_address_step { 0 };
		size_t dummy_read_step { 0 };
	public:
		/*
		* 	@brief 	This function inits all bq in a stach
		* 	@retval	HAL_BUSY when init is in progress is in progress, HAL_OK when done
		*/
		HAL_StatusTypeDef init_uart()
		{
			constexpr uint32_t rx_timeout_baudblocks = to_bb(t_rx_timeout, default_baudrate);
	
			HAL_UART_ReceiverTimeout_Config(huart, rx_timeout_baudblocks);

			return HAL_OK;
		}

		HAL_StatusTypeDef init_stack()
		{
			using namespace Utils;
			switch(init_state)
			{
			case 0:
			{
				if(wake_up() == HAL_BUSY) return HAL_BUSY;
				else init_state = 1;
			} break;
			case 1:
			{
				Control1 ctrl1;
				ctrl1.send_wake = true;
				uint8_t data = tob(ctrl1);
				if(write<ReqType::Single>(&data, 1, sta<Control1>()) == HAL_BUSY) return HAL_BUSY;
				else init_state = 2;
			} break;
			case 2:
			{
				uint8_t data = 0x00;
				if(write<ReqType::Stack>(&data, 1, 0x343 + dummy_write_step) == HAL_BUSY) return HAL_BUSY;
				else if(dummy_write_step < 8) dummy_write_step++;
				else init_state = 3;
			} break;
			case 3:
			{
				uint8_t data = 0x01;
				if(write<ReqType::Broadcast>(&data, 1, 0x309) == HAL_BUSY) return HAL_BUSY;
				else init_state = 4;
			} break;
			case 4:
			{
				uint8_t data = auto_address_step;
				if(write<ReqType::Stack>(&data, 1, 0x306) == HAL_BUSY) return HAL_BUSY;
				else if(auto_address_step < CHAIN_SIZE) auto_address_step++;
				else init_state = 5;
			} break;
			case 5:
			{
				uint8_t data = 0x02;
				if(write<ReqType::Stack>(&data, 1, 0x308) == HAL_BUSY) return HAL_BUSY;
				else init_state = 6;
			} break;
			case 6:
			{
				uint8_t data = 0x03;
				if(write<ReqType::Single>(&data, 1, 0x308, CHAIN_SIZE) == HAL_BUSY) return HAL_BUSY;
				else init_state = 7;
			} break;
			case 7:
			{
				uint8_t data[CHAIN_SIZE] { 0x00 };
				if(read<ReqType::Stack>(data, 1, 0x343 + dummy_read_step) == HAL_BUSY) return HAL_BUSY;
				else if(dummy_read_step < 8) dummy_read_step++;
				else init_state = 8;
			} break;
			case 8:
			{
				uint8_t data[CHAIN_SIZE] { 0x00 };
				if(read<ReqType::Stack>(data, 1, 0x306) == HAL_BUSY) return HAL_BUSY;
				else init_state = 9;
			}
			case 9:
			{
				uint8_t data { 0x00 };
				if(read<ReqType::Stack>(&data, 1, 0x2001) == HAL_BUSY) return HAL_BUSY;
				else 
				{
					if(data != 0x14) Error_Handler();
					init_state = 10;
				}
			}
			default:
			{
				init_state = 0;
				dummy_write_step = 0;
				auto_address_step = 0;
				dummy_read_step = 0;
				return HAL_OK;
			} break;
			}
			return HAL_BUSY;
		}

	private:
		bool wake_up_done { false };
	public:
		/*
		* 	@brief 	Wake up function for BQ79600 IC, this functions tries to hold the MOSI line
		*			for aprox ~2.5ms
		* 	@retval	HAL_BUSY when wakeing up is in progress, HAL_OK when done
		*/
		HAL_StatusTypeDef wake_up()
		{
			/* prevent override during checks */
			volatile UartState state (huart->gState);

			if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

			if(state.tx_busy or state.uart_busy) return HAL_BUSY;

			if(wake_up_done) { wake_up_done = false; return HAL_OK; }

			huart->UserData = (void*)this;

			out.at(0) = 0b1000'0001;

			if(HAL_UART_DeInit(huart) != HAL_OK) Error_Handler();
			huart->Init.BaudRate = baudrate_wakeup;
			if(HAL_UART_Init(huart) != HAL_OK) Error_Handler();

			// FIXME: might not work
			callback_write = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;

				volatile UartState state (huart->gState);

				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				if(HAL_UART_DeInit(huart) != HAL_OK) Error_Handler();
				huart->Init.BaudRate = default_baudrate;
				if(HAL_UART_Init(huart) != HAL_OK) Error_Handler();

				huart->TxCpltCallback = HAL_UART_TxCpltCallback;

				bq->wake_up_done = true;
			};

			huart->TxCpltCallback = callback_write;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), 1) != HAL_OK) Error_Handler();

			return HAL_BUSY;
		}

	private:
		bool write_done { false };
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
		template<Utils::ReqType REQ_TYPE> requires (Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef write(uint8_t *data, size_t size, uint16_t reg_address, uint8_t address = 0)
		{
			/* prevent override during checks? */
			volatile UartState state (huart->gState);

			if(not state.init_done or state.status == UartStatus::Error) Error_Handler();
			if(state.tx_busy or state.uart_busy) return HAL_BUSY;
			if(write_done) { write_done = false; return HAL_OK; }

			huart->UserData = (void*)this;
			
			out.at(0) = Utils::init_byte_write<REQ_TYPE>(size);

			if(address > 0x3f) address = 0x3f;
			out.at(1) = address;

			out.at(2) = (uint8_t)reg_address >> 8;
			out.at(3) = (uint8_t)reg_address;

			std::copy(data, data + size, out.begin() + 4);

			uint16_t crc = crc16.fast(out.begin(), size + 4);
			out.at(size + 4) = (uint8_t)(crc >> 8);
			out.at(size + 5) = (uint8_t)(crc);

			callback_write = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;

				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				bq->write_done = true;
				huart->TxCpltCallback = HAL_UART_TxCpltCallback;
				huart->UserData = nullptr;
			};

			huart->TxCpltCallback = callback_write;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), size + 6) != HAL_OK) Error_Handler();

			return HAL_BUSY;
		}

	private:
		bool read_done { false };
		size_t read_size { 0 };
	public:
		/*
		* 	@brief 	Send `data` of `size` to a device at `address`. If data was send before this function
		*			can be called without any parameters to check the `writeSingle` state: `HAL_BUSY` (sending)
		*			or `HAL_OK` (done)
		* 	@tparam	`REG` first register to write, if more registers are writen they need to have an incrementing address
		*	@tparam	`REQ_TYPE` write type
		* 	@param 	`data`
		*	@param 	`address` address of a device to be written to - assumes 0
		* 	@retval	HAL_BUSY when writeSingle is in progress, HAL_OK when done or caller provided no data/size
		*/
		template<Utils::ReqType REQ_TYPE> requires (Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef read(uint8_t *data, size_t size, uint16_t reg_address, uint8_t address = 0)
		{
			/* prevent override during checks? */
			volatile UartState state (huart->gState);

			if(not state.init_done or state.status == UartStatus::Error) Error_Handler();
			if(state.tx_busy or state.uart_busy) return HAL_BUSY;
			if(read_done) 
			{ 
				read_done = false; 
				size_t count = read_count<REQ_TYPE>();
				auto it_begin = in.begin();
				auto it_end = in.begin() + size + 6;
				auto it_data = data;
				for(size_t i = 0; i < count; i++)
				{
					std::copy(it_begin, it_end, it_data);
					it_begin += size + 6;
					it_end += size + 6;
					it_data += size;
				}
				return HAL_OK; 
			}

			huart->UserData = (void*)this;
			read_size = size;
			
			out.at(0) = Utils::init_byte_read<REQ_TYPE>();

			if(address > 0x3f) address = 0x3f;
			out.at(1) = address;

			out.at(2) = (uint8_t)reg_address >> 8;
			out.at(3) = (uint8_t)reg_address;

			out.at(4) = size;

			uint16_t crc = crc16.fast(out.begin(), 5);
			out.at(5) = (uint8_t)(crc >> 8);
			out.at(6) = (uint8_t)(crc);

			callback_write = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;
				
				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				if(HAL_UART_Receive_DMA(huart, (uint8_t*)bq->in.begin(), (bq->read_size + 6) * read_count<REQ_TYPE>()) != HAL_OK) Error_Handler();
				
				huart->TxCpltCallback = HAL_UART_TxCpltCallback;
			};

			callback_read = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;

				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				bq->read_done = true;
				huart->RxCpltCallback = HAL_UART_RxCpltCallback;
				huart->UserData = nullptr;
			};

			huart->RxCpltCallback = callback_read;
			huart->TxCpltCallback = callback_write;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), 7) != HAL_OK) Error_Handler();
			//if(HAL_UART_Receive_DMA(huart, (uint8_t*)in.begin(), (size + 6) * read_count<REQ_TYPE>()) != HAL_OK) Error_Handler();

			return HAL_BUSY;
		}

	};
}

#endif /* INC_BQ796XX_HPP_ */

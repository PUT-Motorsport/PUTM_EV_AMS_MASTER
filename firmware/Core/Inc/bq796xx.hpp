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
#include "cassert"

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
		
		enum struct Channel : uint8_t 
		{
			_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
		};

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
			AdcCtrl1(MainMode main_mode = MainMode::No, bool main_go = false, bool lpf_cell_en = false, bool lpf_bb_en = false) : main_mode(main_mode), main_go(main_go), lpf_cell_en(lpf_cell_en), lpf_bb_en(lpf_bb_en) { }
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

		struct __packed OVThresh : public IReg, IAddress<0x0009>
		{
		public:
			uint8_t treshhold : 6;
		private:
			uint8_t reserved : 2;
		};

		struct __packed UVThresh : public IReg, IAddress<0x000A>
		{
		public:
			uint8_t treshhold : 6;
		private:
			uint8_t reserved : 2;
		};

		struct __packed UVDisable1 : public IReg, IAddress<0x000C>
		{
			bool cell9 : 1 { false };
			bool cell10 : 1 { false };
			bool cell11 : 1 { false };
			bool cell12 : 1 { false };
			bool cell13 : 1 { false };
			bool cell14 : 1 { false };
			bool cell15 : 1 { false };
			bool cell16 : 1 { false };
		};

		struct __packed UVDisable2 : public IReg, IAddress<0x000D>
		{
			bool cell1 : 1 { false };
			bool cell2 : 1 { false };
			bool cell3 : 1 { false };
			bool cell4 : 1 { false };
			bool cell5 : 1 { false };
			bool cell6 : 1 { false };
			bool cell7 : 1 { false };
			bool cell8 : 1 { false };
		};

		struct __packed OTUTTresh : public IReg, IAddress<0x000B>
		{
			uint8_t ut_tresh : 3;
			uint8_t ot_tresh : 5;
		};

		enum struct OVUVMode : uint8_t
		{
			Stop,
			RoundRobin,
			Once,
			SingleChannel
		};

		struct __packed OVUVCtrl : public IReg, IAddress<0x032C>
		{
			OVUVCtrl(OVUVMode ovuv_mode = OVUVMode::Stop, bool ovuv_go = false, uint8_t ovuv_lock = 0, uint8_t vcbdone_thr_lock = 0) : ovuv_mode(ovuv_mode), ovuv_go(ovuv_go), ovuv_lock(ovuv_lock), vcbdone_thr_lock(vcbdone_thr_lock) { }
			OVUVMode ovuv_mode : 2 { OVUVMode::Stop };
			bool ovuv_go : 1 { false };
			uint8_t ovuv_lock : 4 { 0 };
			uint8_t vcbdone_thr_lock : 1 { 0 };
		};

		enum struct ReqType : uint8_t
		{
			Single 		= 0b00,
			Stack		= 0b01,
			Broadcast	= 0b10
		};

		template<ReqType REQ_TYPE> 
		concept IsReqType = 0 <= (uint8_t)REQ_TYPE and (uint8_t)REQ_TYPE <= 2;

		template<ReqType REQ_TYPE> requires IsReqType<REQ_TYPE>
		uint8_t init_byte_write(uint8_t data_size)
		{
			// uint8_t rsvd = 0b0000'0000;
			uint8_t frame_type = 0b1'0000000;
			uint8_t req_type = 0b0'001'0000 | (uint8_t)REQ_TYPE << 5;
			if(1 > data_size or data_size > 8) Error_Handler();
			data_size--;

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
	
	template<size_t STACK_SIZE> requires ( STACK_SIZE <= 64 )
	class Bq796xx
	{
	private: 
		/* Bq79600 has a preset baud rate of 1Mbps [bps] */
		constexpr static inline uint32_t default_baudrate = 1'000'000;
		/* t hold wakeup, in range <2500, 3000> [us] */
		constexpr static inline uint32_t t_wakeup = 2'600;
		/* rx timeout [us] (2 * defualt) */
		constexpr static inline uint32_t t_rx_timeout = 500;
		/* baudrate for init [bps], assume 1 bit high before, 6 bits of low time and 1 bit high after, this should create the required pattern */
		constexpr static inline uint32_t baudrate_wakeup = (uint32_t)(1'000'000.0 / (double)t_wakeup * 6.0);
		/* uV per bit */
		constexpr static inline double v_lsb_adc = 190.73e-6;

		/*
		* 	@brief 	staticly calculate the number of devices being read from
		*	@return	number of devices being read from
		*/
		template<Utils::ReqType REQ_TYPE> requires Utils::IsReqType<REQ_TYPE>
		static consteval size_t read_count()
		{
			switch (REQ_TYPE)
			{
			case Utils::ReqType::Single:
				return 1;
			case Utils::ReqType::Stack:
				return STACK_SIZE;
			case Utils::ReqType::Broadcast:
				return STACK_SIZE + 1;
			}
			Utils::throw_consteval_failure("WRONG");
		}
		
		/*
		*	@brief `enum struct` used for internal state keeping 
		*/
		enum struct State
		{
			Idle,
			InProgress,
			Done
		};

		/*
		*	@brief 	convert us and baudrate to needed baudblocks
		*	@param `time` in us
		*	@param `baudrate` in bps
		*	@return	baudblocks
		*/
		static consteval uint32_t to_bb(uint32_t time, uint32_t baudrate) { return (uint32_t)((double)time * (double)baudrate / 1'000'000.0); }

	public:
		/*
		*	@brief init Bq796xx, set receiver timeout to t_rx_timeout (~300 us) for the uart handler to for data in IT or DMA mode
		*	@param `huart` uart handle
		*/
		Bq796xx(UART_HandleTypeDef *huart) : huart(huart) { }
	private:
		UART_HandleTypeDef *huart;

		volatile pUART_CallbackTypeDef callback_read { nullptr };
		volatile pUART_CallbackTypeDef callback_write { nullptr };
		volatile pUART_CallbackTypeDef callback_error { nullptr };

		/* for now leave the size at 256 */
		std::array<uint8_t, 256> out { 0 };
		/* for now leave the size at 256 */
		std::array<uint8_t, 256> in { 0 };

	public:
		/*
		* 	@brief 	This function inits other uart communication, call this function firsts
		* 	@retval	HAL_OK
		*/
		HAL_StatusTypeDef init_uart()
		{
			constexpr uint32_t rx_timeout_baudblocks = to_bb(t_rx_timeout, default_baudrate);
	
			HAL_UART_ReceiverTimeout_Config(huart, rx_timeout_baudblocks);
			HAL_UART_EnableReceiverTimeout(huart);
			//SET_BIT(huart->Instance->CR1, USART_CR1_RTOIE);

			return HAL_OK;
		}
	private:
		size_t init_stack_state { 0 };
		size_t dummy_write_step { 0 };
		size_t auto_address_step { 0 };
		size_t dummy_read_step { 0 };

		uint32_t tick { 0 };
	public:
		/*
		* 	@brief 	This function inits whole stack communication
		* 	@retval	HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
		*/
		HAL_StatusTypeDef init_stack()
		{
			using namespace Utils;

			switch(init_stack_state)
			{
			case 0: /* wake up bq79600 */
			{
				if(wake_up() == HAL_BUSY) return HAL_BUSY;
				else { init_stack_state = 1; tick = HAL_GetTick(); }
			} break;
			case 1: /* make sure wait lasted at least 4ms*/
			{
				if(HAL_GetTick() - tick < 4) return HAL_BUSY;
				else init_stack_state = 2;
			} break;
			case 2: /* cmd wake up slaves */
			{
				Control1 ctrl1;
				ctrl1.send_wake = true;
				uint8_t data = tob(ctrl1);
				if(write<ReqType::Single>(&data, 1, sta<Control1>()) == HAL_BUSY) return HAL_BUSY;
				else { init_stack_state = 3; tick = HAL_GetTick(); }
			} break;
			case 3: /* make sure wait lasted at least 15ms*/
			{
				if(HAL_GetTick() - tick < 15) return HAL_BUSY;
				else init_stack_state = 4;
			} break;
			case 4: /* dummy write 0x00, sync internal dlls */
			{
				uint8_t data = 0x00;
				if(write<ReqType::Broadcast>(&data, 1, 0x343 + dummy_write_step) == HAL_BUSY) return HAL_BUSY;
				else if(dummy_write_step < 8) dummy_write_step++;
				if(dummy_write_step == 8) init_stack_state = 5;
			} break;
			case 5: /* enable auto adressing */
			{
				uint8_t data = 0x01;
				if(write<ReqType::Broadcast>(&data, 1, 0x309) == HAL_BUSY) return HAL_BUSY;
				else init_stack_state = 6;
			} break;
			case 6: /* auto addressing */
			{
				uint8_t data = auto_address_step;
				if(write<ReqType::Broadcast>(&data, 1, 0x306) == HAL_BUSY) return HAL_BUSY;
				else if(auto_address_step < STACK_SIZE) auto_address_step++;
				if(auto_address_step == STACK_SIZE) init_stack_state = 7;
			} break;
			case 7: /* set bq7961x as stack device */
			{
				uint8_t data = 0x02;
				if(write<ReqType::Broadcast>(&data, 1, 0x308) == HAL_BUSY) return HAL_BUSY;
				else init_stack_state = 8;
			} break;
			case 8: /* set which bq is last */
			{
				uint8_t data = 0x03;
				if(write<ReqType::Single>(&data, 1, 0x308, STACK_SIZE) == HAL_BUSY) return HAL_BUSY;
				else init_stack_state = 9;
			} break;
			case 9: /* dummy read sync internal dlls */
			{
				uint8_t data[STACK_SIZE] { 0x00 };
				if(read<ReqType::Stack>(data, 1, 0x343 + dummy_read_step) == HAL_BUSY) return HAL_BUSY;
				else if(dummy_read_step < 8) dummy_read_step++;
				if(dummy_read_step == 8) init_stack_state = 10;
			} break;
			case 10: /* verify adresses */
			{
				uint8_t data[STACK_SIZE] { 0x00 };
				if(read<ReqType::Stack>(data, 1, 0x306) == HAL_BUSY) return HAL_BUSY;
				else 
				{
					// TODO: internal status error
					// for(size_t i = 0; i < STACK_SIZE; i++) if(data[i] != i + 1) ;
					init_stack_state = 11;
				}
			} break;
			case 11: /* check sth on bq79600 */
			{
				uint8_t data { 0x00 };
				if(read<ReqType::Single>(&data, 1, 0x2001) == HAL_BUSY) return HAL_BUSY;
				else 
				{
					// TODO: internal status error
					// if(data != 0x14) return HAL_ERROR;
					init_stack_state = 12;
				}
			} break;
			default: /* '12' finish init */
			{
				init_stack_state = 0;
				dummy_write_step = 0;
				auto_address_step = 0;
				dummy_read_step = 0;
				return HAL_OK;
			} break;
			}

			return HAL_BUSY;
		}
	private:
		size_t init_voltages_measurement_state { 0 };	
	public:
		HAL_StatusTypeDef init_voltage_measurement()
		{
			using namespace Utils;

			switch(init_voltages_measurement_state)
			{
			case 0: /* set active cells in series */
			{
				/* set active cells to 14S */
				uint8_t data = 0x8; 
				if(write<ReqType::Broadcast>(&data, 1, 0x0003) == HAL_BUSY) return HAL_BUSY;
				else init_voltages_measurement_state = 1;
			} break;
			case 1: /* set control */
			{
				/* set adc continous, start conversion, enable lpf */
				uint8_t data = tob(AdcCtrl1(MainMode::Continous, true, true)); 
				if(write<ReqType::Broadcast>(&data, 1, 0x030D) == HAL_BUSY) return HAL_BUSY;
				else init_voltages_measurement_state = 2;
			} break;
			default:
			{
				init_voltages_measurement_state = 0;
				return HAL_OK;
			} break;
			}

			return HAL_BUSY;
		}
	public:
		/* 	
		*	@brief	set undervoltage and overvoltage protection
		*	@param	`undervoltage` in mV, must be between 1200 and 3100
		*	@param	`overvoltage` in mV, must be between 2700 and 4475
		*	@retval HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
		*/
		HAL_StatusTypeDef set_ovuv(uint32_t undervoltage, uint32_t overvoltage)
		{
			using namespace Utils;

			assert(1200 <= undervoltage and undervoltage <= 3100);
			assert(2700 <= overvoltage and overvoltage <= 4475);

			uint8_t uv = (uint8_t)((undervoltage - 1200 / 50) & 0x3f);
			uint8_t ov = (uint8_t)((overvoltage - 2700 / 25) & 0x3f);

			uint8_t data[] = { ov, uv };

			return write<ReqType::Stack>(data, 2, sta<OVThresh>());
		}
	public:
		/* 	
		*	@brief	disable undervoltage detection on selected channels, every call overrides past calls
		*	@param 	`channels` array with channel numbers
		*	@param	`size` size of array
		*	@retval HAL_OK when done, HAL_BUSY when init in progress, HAL_ERROR on fail
		*/
		HAL_StatusTypeDef set_uv_disable(Utils::Channel *channels, size_t size)
		{
			using namespace Utils;

			uint16_t buffer;
			for(size_t i = 0; i < size; i++)
			{
				buffer |= (1 << (uint8_t)channels[i]);
			}

			uint8_t data[] = { (uint8_t)(buffer >> 8 & 0xff), (uint8_t)(buffer & 0xff) };
			
			return write<ReqType::Stack>(data, 2, sta<UVDisable1>());
		}
	public:
		HAL_StatusTypeDef set_ovuv_enable()
		{
			using namespace Utils;

			/* set ovuv mode to round robin and enable ovuv */
			uint8_t data[] = { tob(OVUVCtrl(OVUVMode::RoundRobin, true)) };

			return write<ReqType::Stack>(data, 1, 0x032C);
		}
	private:
		/*
		*	@brief struct for local stack status storage
		*/
		struct StackDeviceStatus
		{
			std::array<bool, 16> ovuv;
		};
		std::array<StackDeviceStatus, STACK_SIZE> stack_device_status;
	public:
		/*
		*	@brief poll stack status to local storage
		*/
		HAL_StatusTypeDef update_status()
		{
			using namespace Utils;

			constexpr data_size = 4;

			uint8_t buffer[data_size * STACK_SIZE] { 0 };

			/* read ov1/2 and uv1/2, 0x053C, address of FAULT_OV1, yes i started getting lazy */
			HAL_StatusTypeDef status = read<ReqType::Stack>(buffer, data_size, 0x053C);

			if(status != HAL_OK) return status;
			
			for(size_t idev = 0; idev < STACK_SIZE; idev++)
			{
				size_t offset = data_size * idev;
				uint16_t ov_tmp = *(uint16_t*)buffer[0 + offset];
				uint16_t uv_tmp = *(uint16_t*)buffer[2 + offset];
				uint16_t ovuv_tmp = ov_tmp | uv_tmp;

				for(size_t ich = 0; ich < 16; ich++)
				{
					size_t bit_index = 1 << ich;
					stack_device_status[idev].ovuv[ich] = (bool)(ovuv_tmp & bit_index);
				}
			}

			return HAL_OK;
		}
	private:
		/*
		*	@brief struct for local stack data storage
		*/
		struct StackDeviceData
		{
			std::array<float, 16> voltages;
		};
		std::array<StackDeviceData, STACK_SIZE> stack_device_data;
	public:
		/*
		*	@brief poll stack voltages to local storage
		*/
		HAL_StatusTypeDef update_voltages()
		{
		 	using namespace Utils;

			/* 16 cells * 2 bytes */
			constexpr size_t data_count = 16 * 2;
		 	uint8_t buffer[data_count * STACK_SIZE] { 0 };

		 	/* read ovoltages, address of VCELL16_HI */
		 	HAL_StatusTypeDef status = read<ReqType::Stack>(buffer, data_count, 0x0568);

		 	if(status != HAL_OK) return status;
			
			/* voltages */
			for(size_t idev = 0; idev < STACK_SIZE; idev++)
			{
				for(size_t ich = 0; ich < 16; ich++)
				{
					size_t index = idev * data_count + ich * 2;
					int16_t volt = ((uint16_t)(buffer[index]) << 8 | (uint16_t)(buffer[index + 1]));
					
					stack_device_data[idev].voltages[15 - ich] = -(~volt + 1) * v_lsb_adc;
				}
			}
			
		 	return HAL_OK;
		}
	public:

		/*
		*	@brief	get overvolatge/undervoltage register status
		*	@param	`data` array to write the results to, true means undervoltage or overvoltage event on channel has ocurred
		*	@param 	`size` size of required data, this value shoud be between, 1 an 16
		*/
		// HAL_StatusTypeDef get_ovuv_status(bool *data[CHAINS_SIZE], size_t size)
		// {
		// 	uint8_t data[4 * STACK_SIZE];
		// }
	private:
		/*
		*	@brief 	wake up state
		*/
		State wake_up_state { State::Idle };
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

			if(state.tx_busy or state.uart_busy or wake_up_state == State::InProgress) return HAL_BUSY;

			if(wake_up_state == State::Done) { wake_up_state = State::Idle; return HAL_OK; }

			huart->UserData = (void*)this;

			/* uart sends lsb first, this sequence includes start bit for a total of '6' bits*/
			out.at(0) = 0b1110'0000;

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

				bq->wake_up_state = State::Done;
				huart->TxCpltCallback = HAL_UART_TxCpltCallback;
			};

			huart->TxCpltCallback = callback_write;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), 1) != HAL_OK) Error_Handler();

			wake_up_state = State::InProgress;
			return HAL_BUSY;
		}

	private:
		State write_state { State::Idle };
	public:
		/*
		* 	@brief 	Send `data` of `size` to a device at `address` in `REQ_TYPE` mode. All registers are 1 byte in len, 
		*			so any write with size > 1 is automaticaly interpreted as serial write begining at address `reg_address`.
		*			If data was send before this function can be called without any parameters to check the `writeSingle` state: 
		*			`HAL_BUSY` (sending) or `HAL_OK` (done)
		*	@tparam	`REQ_TYPE` write type
		* 	@param 	`data` data, cant be largen than 8 bytes
		*	@param	`size` size of data cant be larger than 8
		*	@param 	`address` address of a device to be written to - assumes 0
		* 	@retval	HAL_BUSY when writeSingle is in progress, HAL_OK when done
		*/
		template<Utils::ReqType REQ_TYPE> requires (Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef write(uint8_t *data, size_t size, uint16_t reg_address, uint8_t address = 0)
		{
			/* prevent override during checks? */
			volatile UartState state (huart->gState);

			if(data == nullptr or size == 0) Error_Handler();
			if(not state.init_done or state.status == UartStatus::Error) Error_Handler();
			else if(state.tx_busy or state.uart_busy or write_state == State::InProgress) return HAL_BUSY;
			else if(write_state == State::Done) { write_state = State::Idle; return HAL_OK; }

			huart->UserData = (void*)this;
			
			size_t i = 0;
			out.at(i++) = Utils::init_byte_write<REQ_TYPE>(size);

			if constexpr(REQ_TYPE == Utils::ReqType::Single) out.at(i++) = address;

			out.at(i++) = (uint8_t)(reg_address >> 8);
			out.at(i++) = (uint8_t)(reg_address);

			std::copy(data, data + size, out.begin() + i);

			uint16_t crc = crc16.fast(out.begin(), size + i);
			out.at(size + i++) = (uint8_t)(crc >> 8);
			out.at(size + i++) = (uint8_t)(crc);

			callback_write = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;

				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				bq->write_state = State::Done;
				huart->TxCpltCallback = HAL_UART_TxCpltCallback;
				huart->UserData = nullptr;
			};

			huart->TxCpltCallback = callback_write;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), size + i) != HAL_OK) Error_Handler();

			write_state = State::InProgress;
			return HAL_BUSY;
		}

	private:
		State read_state { State::Idle };
		size_t read_size { 0 };
		uint32_t read_start { 0 };
	public:
		/*
		* 	@brief 	read `data` of `size` from a device at `address` in `REQ_TYPE` mode. All registers are 1 byte in len, 
		*			so any read with size > 1 is automaticaly interpreted as serial read begining at address `reg_address`.
		*			This function doesn't block it's caller however it will return HAL_BYSY, when reading data is in proggress 
		*			so it's up to the user to hadle it properly. This function times out after 1ms.
		*	@tparam	`REQ_TYPE` read type, Single, Stack or Broadcast
		* 	@param 	`data` copies the received data to provided container, when new data was received. If for any reason data received
		*			was coruppted or not received it will not be coppied over to the procided buffer. Data should point to a buffer of an
		*			appropriate size - size for single read, size * STACK_SIZE for stack read
		*	@param 	`size` number of registers to read not the size of the array!
		*	@param 	`address` address of a device to be written to in signle mode, assumes 0. In other modes it is ignored
		* 	@retval	HAL_BUSY when read is in progress, HAL_OK when done or caller provided no data/size, HAL_TIMEOUT when read operation wasn't
		*			properly executed.
		*/
		template<Utils::ReqType REQ_TYPE> requires (Utils::IsReqType<REQ_TYPE>)
		HAL_StatusTypeDef read(uint8_t *data, size_t count, uint16_t reg_address, uint8_t address = 0)
		{
			/* prevent override during checks? */
			volatile UartState state (huart->gState);

			if(data == nullptr or count == 0) Error_Handler();
			if(not state.init_done or state.status == UartStatus::Error or count > 128 or count < 1) Error_Handler();
			else if(HAL_GetTick() - read_start > 1 and read_state == State::InProgress)
			{
				if(HAL_UART_AbortReceive(huart) != HAL_OK) Error_Handler();

				read_state = State::Idle; 
				constexpr size_t size = read_count<REQ_TYPE>();
				std::fill(data, data + count * size, 0);
				return HAL_TIMEOUT;
			}
			else if(state.tx_busy or state.uart_busy or read_state == State::InProgress) return HAL_BUSY;
			else if(read_state == State::Done) 
			{ 
				read_state = State::Idle; 
				constexpr size_t size = read_count<REQ_TYPE>();
				auto it_data_begin = in.begin() + 4;
				auto it_data_end = in.begin() + 4 + count;
				auto it_data = data;
				for(size_t i = 0; i < size; i++)
				{
					std::copy(it_data_begin, it_data_end, it_data);
					it_data_begin += count + 6;
					it_data_end += count + 6;
					it_data += count;
				}
				return HAL_OK; 
			}

			huart->UserData = (void*)this;
			read_size = count;
			
			size_t i = 0;
			out.at(i++) = Utils::init_byte_read<REQ_TYPE>();

			if constexpr(REQ_TYPE == Utils::ReqType::Single) out.at(i++) = address;

			out.at(i++) = (uint8_t)(reg_address >> 8);
			out.at(i++) = (uint8_t)(reg_address);

			out.at(i++) = count - 1;

			uint16_t crc = crc16.fast(out.begin(), i);
			out.at(i++) = (uint8_t)(crc >> 8);
			out.at(i++) = (uint8_t)(crc);

			callback_write = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;
				
				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				HAL_UART_EnableReceiverTimeout(huart);
				if(HAL_UART_Receive_DMA(huart, (uint8_t*)bq->in.begin(), (bq->read_size + 6) * read_count<REQ_TYPE>()) != HAL_OK) Error_Handler();
				
				huart->TxCpltCallback = HAL_UART_TxCpltCallback;
			};

			callback_read = [](UART_HandleTypeDef* huart)
			{
				if(huart->UserData == nullptr) Error_Handler();
				Bq796xx *bq = (Bq796xx*)huart->UserData;

				volatile UartState state (huart->gState);
				if(not state.init_done or state.status == UartStatus::Error) Error_Handler();

				bq->read_state = State::Done;
				huart->RxCpltCallback = HAL_UART_RxCpltCallback;
				huart->UserData = nullptr;
			};

			huart->TxCpltCallback = callback_write;
			huart->RxCpltCallback = callback_read;

			if(HAL_UART_Transmit_DMA(huart, (uint8_t*)out.begin(), i) != HAL_OK) Error_Handler();
			//if(HAL_UART_Receive_DMA(huart, (uint8_t*)in.begin(), (size + 6) * read_count<REQ_TYPE>()) != HAL_OK) Error_Handler();

			read_start = HAL_GetTick();
			read_state = State::InProgress;
			return HAL_BUSY;
		}
	};
}

#endif /* INC_BQ796XX_HPP_ */

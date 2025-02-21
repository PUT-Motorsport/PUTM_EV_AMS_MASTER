/*
 * bq79616.hpp
 *
 *  Created on: Feb 18, 2025
 *      Author: lenovo
 */

#ifndef INC_BQ79616_HPP_
#define INC_BQ79616_HPP_

#include "main.h"
#include "concepts"
#include "utility"

// # define EMPTY(...)
// # define DEFER(...) __VA_ARGS__ EMPTY()
// # define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
// # define EXPAND(...) __VA_ARGS__

// #define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
// #define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
// #define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
// #define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
// #define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
// #define EVAL5(...) EVAL6(EVAL6(EVAL6(__VA_ARGS__)))
// #define EVAL6(...) EVAL7(EVAL7(EVAL7(__VA_ARGS__)))
// #define EVAL7(...) __VA_ARGS__

// #define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
// #define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

// #define DEC(x) PRIMITIVE_CAT(DEC_, x)
// #define DEC_0 0
// #define DEC_1 0
// #define DEC_2 1
// #define DEC_3 2
// #define DEC_4 3
// #define DEC_5 4
// #define DEC_6 5
// #define DEC_7 6
// #define DEC_8 7
// #define DEC_9 8
// #define DEC_10 9
// #define DEC_11 10
// #define DEC_12 11
// #define DEC_13 12
// #define DEC_14 13
// #define DEC_15 14
// #define DEC_16 15
// #define DEC_17 16
// #define DEC_18 17
// #define DEC_19 18

// #define CHECK_N(x, n, ...) n
// #define CHECK(...) CHECK_N(__VA_ARGS__, 0,)

// #define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
// #define NOT_0 ~, 1,

// #define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
// #define COMPL_0 1
// #define COMPL_1 0

// #define BOOL(x) COMPL(NOT(x))

// #define IIF(c) PRIMITIVE_CAT(IIF_, c)
// #define IIF_0(t, ...) __VA_ARGS__
// #define IIF_1(t, ...) t

// #define IF(c) IIF(BOOL(c))

// #define EAT(...)
// #define EXPAND(...) __VA_ARGS__
// #define WHEN(c) IF(c)(EXPAND, EAT)

// #define REPEAT(count, macro, ...) \
//     WHEN(count) \
//     ( \
//         OBSTRUCT(REPEAT_INDIRECT) () \
//         ( \
//             DEC(count), macro, __VA_ARGS__ \
//         ) \
//         OBSTRUCT(macro) \
//         ( \
//             DEC(count), __VA_ARGS__ \
//         ) \
//     )
// #define REPEAT_INDIRECT() REPEAT



namespace Bq
{
	namespace Utils
	{
		struct IReg { };

		template<typename T>
		concept IsReg = std::is_base_of<IReg, T>::value and not std::is_polymorphic<T>::value and sizeof(T) == 1;
		void throw_consteval_failure(char const*);

		template<typename T, uint16_t A> requires IsReg<T>
		uint16_t consteval sta() { throw_consteval_failure("failed to evaluate sta"); return A; }


#define ADD_STA(CLASS, ADDRESS) \
template<> uint16_t consteval Utils::sta<CLASS, ADDRESS>() { return ADDRESS; }

#define ADD_STA_RANGE(CLASS, N, ADDR_START) \


		struct Dir0AddrOtp : public IReg
		{
			uint8_t undef { 0x00 };
		};
		ADD_STA(Dir0AddrOtp, 0x0000);

		struct Dir1AddrOtp : public IReg
		{
			uint8_t undef { 0x00 };
		};
		ADD_STA(Dir1AddrOtp, 0x0001);

		template<size_t T>
		concept IsMiscReg = T >= 1 and T <= 9;

		template<size_t MiscReg> requires IsMiscReg<MiscReg>
		struct CustMisc : public IReg
		{
			uint8_t data;
		};

		struct Dir0Addr : public IReg
		{
			uint8_t undef;
		};

		struct Dir1Addr : public IReg
		{
			uint8_t undef;
		};

		struct PartId : public IReg
		{
			uint8_t part_id;
		};

		struct DevRevId : public IReg
		{
			uint8_t undef;
		};

		template<size_t T>
		concept IsDieIdReg = T >= 1 and T <= 9;

		template<size_t DieIdReg> requires IsDieIdReg<DieIdReg>
		struct DieId : public IReg
		{
			uint8_t id;
		};

		struct DevConf
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

		enum struct NumCell : uint8_t
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

		struct ActiveCell : public IReg
		{
		public:
			NumCell num_cell : 4 { NumCell::_16S };
		private:
			uint8_t spare : 4 { 0b0000 };
		};

		struct BbvcPosn1 : public IReg
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

		struct BbvcPosn2 : public IReg
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

		enum struct SleepTime : uint8_t
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

		enum struct TWarnThr : uint8_t
		{
			_85C,
			_95C,
			_105C,
			_115C
		};

		struct PwrTransitConf : public IReg
		{
		public:
			SleepTime slp_time : 3 { 0b000 };
			TWarnThr twarn_thr : 2 { 0b11 };
		private:
			uint8_t spare : 3 { 0b000 };
		};

		enum struct CtsTime : uint8_t
		{
			No,
			_100ms,
			_2s,
			_10s,
			_10min,
			_30min,
			_1h
		};

		enum struct CtlTime : uint8_t
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

		struct CommTimeoutConf : public IReg
		{
		public:
			CtlTime ctl_time : 3 { 0b000 };
			bool ctl_act : 1 { 0b0 };
			CtsTime cts_time : 3 { 0b000 };
		private:
			uint8_t spare : 1 { 0b0 };
		};

		struct TxHoldOff : public IReg
		{
			uint8_t dly { 0x00 };
		};

		struct StackResponse : public IReg
		{
		public:
			uint8_t dly : 6 { 0b000000 };
		private:
			uint8_t spare : 2 { 0b00 };
		};

		enum struct Loc : uint8_t
		{
			No,
			Undef,
		};

		struct BBPLock : public IReg
		{
		public:
			Loc lock : 5 { 0b00000 };
		private:
			uint8_t spare : 3 { 0b000 };
		};

		struct CommCtl : public IReg
		{
		public:
			bool top_stack : 1 { 0b0 };
			bool stack_dev : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};

		struct Control1 : public IReg
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

		struct Control2 : public IReg
		{
		public:
			bool tsref_en : 1 { 0b0 };
			bool send_hw_reset : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};

		enum struct Byte : uint8_t
		{
			Hi,
			Lo
		};

//		template<size_t T>
//		concept IsByte = T == Byte::Hi or T == Byte::Lo;
//
//		template<size_t CellChannel> requires IsCellChannel<CellChannel>
		struct CustCrcHi: public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct CustCrcRsltHi : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct cust_crc_rlst_lo : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct DiagStat : public IReg
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

		struct AdcStat1 : public IReg
		{
		public:
			bool drdy_main_adc : 1 { 0b0 };
			bool drdy_aux_misc : 1 { 0b0 };
			bool drdy_aux_cell : 1 { 0b0 };
			bool drdy_aux_gpio : 1 { 0b0 };
		private:
			uint8_t reserved : 4 { 0b0 };
		};

		struct AdcStat2 : public IReg
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

		struct GpioStat : public IReg
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

		struct BalStat : public IReg
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

		struct DevStat : public IReg
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

//		enum struct SettleTime
//		{
//			_4_3ms,
//			_2_3ms,
//			_1_3ms,
//			Reserved
//		};
//
//		enum struct CutOff
//		{
//			_6_5Hz,
//			_13Hz,
//			_26Hz,
//			_53Hz,
//			_111
//		};

		struct AdcConf1 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct AdcConf2 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct MainAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct MainAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct AuxAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		struct AuxAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		// shit that will prob be needed 1
		struct AdcCtrl1 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		// shit that will prob be needed 2
		struct AdcCtrl2 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		// shit that will prob be needed 3
		struct AdcCtrl3 : public IReg
		{
			uint8_t undef { 0x00 };
		};

		template<size_t T>
		concept IsCellChannel = T >= 1 and T <= 16;

		template<size_t CellChannel> requires IsCellChannel<CellChannel>
		struct VCellHi : public IReg
		{
			uint8_t result;
		};

		template<size_t CellChannel> requires IsCellChannel<CellChannel>
		struct VCellLo : public IReg
		{
			uint8_t result;
		};

		struct BusBarHi : public IReg
		{
			uint8_t result;
		};

		struct BusBarLo : public IReg
		{
			uint8_t result;
		};

		struct TsrefHi : public IReg
		{
			uint8_t result;
		};

		struct TsrefLo : public IReg
		{
			uint8_t result;
		};

		template<size_t T>
		concept IsGpioChannel = T >= 1 and T <= 8;

		template<size_t GpioChannel> requires IsGpioChannel<GpioChannel>
		struct GpioHi : public IReg
		{
			uint8_t result;
		};

		template<size_t GpioChannel> requires IsGpioChannel<GpioChannel>
		struct GpioLo : public IReg
		{
			uint8_t reuslt;
		};

		struct DieTemp1Hi : public IReg
		{
			uint8_t result;
		};

		struct DieTemp1Lo : public IReg
		{
			uint8_t result;
		};

		struct DieTemp2Hi : public IReg
		{
			uint8_t result;
		};

		struct DieTemp2Lo : public IReg
		{
			uint8_t result;
		};

		struct AuxCellHi : public IReg
		{
			uint8_t result;
		};

		struct AuxCellLo : public IReg
		{
			uint8_t result;
		};

		struct AuxGpioHi : public IReg
		{
			uint8_t result;
		};

		struct AuxGpioLo : public IReg
		{
			uint8_t result;
		};

		struct AuxBatHi : public IReg
		{
			uint8_t result;
		};

		struct AuxBatLo : public IReg
		{
			uint8_t result;
		};

		struct AuxReflHi : public IReg
		{
			uint8_t result;
		};

		struct AuxReflLo : public IReg
		{
			uint8_t result;
		};

		struct AuxVbg2Hi : public IReg
		{
			uint8_t result;
		};

		struct AuxVbg2lLo : public IReg
		{
			uint8_t result;
		};

		struct AuxAvaoRefHi : public IReg
		{
			uint8_t result;
		};

		struct AuxAvaoRefLo : public IReg
		{
			uint8_t result;
		};

		struct AuxAvddRefHi : public IReg
		{
			uint8_t result;
		};

		struct AuxAvddRefLo : public IReg
		{
			uint8_t result;
		};

		struct AuxOvDacHi : public IReg
		{
			uint8_t result;
		};

		struct AuxOvDacLo : public IReg
		{
			uint8_t result;
		};

		struct AuxUvDacHi : public IReg
		{
			uint8_t result;
		};

		struct AuxUvDacLo : public IReg
		{
			uint8_t result;
		};
	}

	namespace Bq79616
	{

	}
}

#undef ADD_STA;

#endif /* INC_BQ79616_HPP_ */

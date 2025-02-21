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


#define ADD_STA(CLASS, ADDRESS) template<> uint16_t consteval Utils::sta<CLASS, ADDRESS>() { return ADDRESS; }


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
		//addr undef

		struct Dir0Addr : public IReg
		{
			uint8_t undef;
		};
		ADD_STA(Dir0Addr, 0x0306);

		struct Dir1Addr : public IReg
		{
			uint8_t undef;
		};
		ADD_STA(Dir1Addr, 0x0307);

		struct PartId : public IReg
		{
			uint8_t part_id;
		};
		ADD_STA(PartId, 0x0500);

		struct DevRevId : public IReg
		{
			uint8_t undef;
		};
		ADD_STA(DevRevId, 0x0e00);

		template<size_t T>
		concept IsDieIdReg = T >= 1 and T <= 9;

		template<size_t DieIdReg> requires IsDieIdReg<DieIdReg>
		struct DieId : public IReg
		{
			uint8_t id;
		};
		//undef

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
		ADD_STA(DevConf, 0x0002);

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
		ADD_STA(ActiveCell, 0x0003);

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
		//undef

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
		//undef

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
		ADD_STA(PwrTransitConf, 0x0018);

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
		ADD_STA(CommTimeoutConf, 0x0019);

		struct TxHoldOff : public IReg
		{
			uint8_t dly { 0x00 };
		};
		ADD_STA(TxHoldOff, 0x001a);

		struct StackResponse : public IReg
		{
		public:
			uint8_t dly : 6 { 0b000000 };
		private:
			uint8_t spare : 2 { 0b00 };
		};
		ADD_STA(StackResponse, 0x0029);

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
		//undef

		struct CommCtrl : public IReg
		{
		public:
			bool top_stack : 1 { 0b0 };
			bool stack_dev : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};
		ADD_STA(CommCtrl, 0x0308);

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
		ADD_STA(Control1, 0x0309);

		struct Control2 : public IReg
		{
		public:
			bool tsref_en : 1 { 0b0 };
			bool send_hw_reset : 1 { 0b0 };
		private:
			uint8_t reserved : 6 { 0b000000 };
		};
		ADD_STA(Control2, 0x030a);

		enum struct Byte : uint8_t
		{
			Hi,
			Lo
		};

		template<size_t T>
		concept IsByte = T == Byte::Hi or T == Byte::Lo;

		template<Byte B> requires IsByte<B>
		struct CustCrc: public IReg
		{
			uint8_t undef { 0x00 };
		};

		template<Byte B> requires IsByte<B>
		struct CustCrcRslt : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

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
		ADD_STA(DiagStat, 0x0526);

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
		ADD_STA(AdcStat1, 0x0527);

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
		ADD_STA(AdcStat2, 0x0528);

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
		ADD_STA(GpioStat, 0x052a);

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
		ADD_STA(GpioStat, 0x052b);

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
		ADD_STA(DevStat, 0x052c);

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
		//undef

		struct AdcConf2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct MainAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct MainAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct AuxAdcCal1 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		struct AuxAdcCal2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		enum struct MainMode : uint8_t
		{
			No,
			Single,
			Continous
		};

		struct AdcCtrl1 : public IReg
		{
		public:
			MainMode main_mode : 2 { 0b00 };
			bool main_go : 1 { 0b0 };
			bool lpf_cell_en : 1 { 0b0 };
			bool lpf_bb_en : 1 { 0b0 };
		private:
			uint8_t reserved : 3 { 0b000 };
		};
		ADD_STA(AdcCtrl1, 0x030d);

		// those are kept at zero so whatever
		struct AdcCtrl2 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		// those are kept at zero so whatever
		struct AdcCtrl3 : public IReg
		{
			uint8_t undef { 0x00 };
		};
		//undef

		template<size_t T>
		concept IsCellChannel = T >= 1 and T <= 16;

		template<Byte B, size_t CellChannel> requires IsCellChannel<CellChannel> and IsByte<B>
		struct VCell : public IReg
		{
			uint8_t result;
		};
		ADD_STA(VCell<Byte::Hi, 16>, 0x0568);
		ADD_STA(VCell<Byte::Lo, 16>, 0x0569);
		ADD_STA(VCell<Byte::Hi, 15>, 0x056a);
		ADD_STA(VCell<Byte::Lo, 15>, 0x056b);
		ADD_STA(VCell<Byte::Hi, 14>, 0x056c);
		ADD_STA(VCell<Byte::Lo, 14>, 0x056d);
		ADD_STA(VCell<Byte::Hi, 13>, 0x056e);
		ADD_STA(VCell<Byte::Lo, 13>, 0x056f);
		ADD_STA(VCell<Byte::Hi, 12>, 0x0570);
		ADD_STA(VCell<Byte::Lo, 12>, 0x0571);
		ADD_STA(VCell<Byte::Hi, 11>, 0x0572);
		ADD_STA(VCell<Byte::Lo, 11>, 0x0573);
		ADD_STA(VCell<Byte::Hi, 10>, 0x0574);
		ADD_STA(VCell<Byte::Lo, 10>, 0x0575);
		ADD_STA(VCell<Byte::Hi, 9>, 0x0576);
		ADD_STA(VCell<Byte::Lo, 9>, 0x0577);
		ADD_STA(VCell<Byte::Hi, 8>, 0x0578);
		ADD_STA(VCell<Byte::Lo, 8>, 0x0579);
		ADD_STA(VCell<Byte::Hi, 7>, 0x057a);
		ADD_STA(VCell<Byte::Lo, 7>, 0x057b);
		ADD_STA(VCell<Byte::Hi, 6>, 0x057c);
		ADD_STA(VCell<Byte::Lo, 6>, 0x057d);
		ADD_STA(VCell<Byte::Hi, 5>, 0x057e);
		ADD_STA(VCell<Byte::Lo, 5>, 0x057f);
		ADD_STA(VCell<Byte::Hi, 4>, 0x0580);
		ADD_STA(VCell<Byte::Lo, 4>, 0x0581);
		ADD_STA(VCell<Byte::Hi, 3>, 0x0582);
		ADD_STA(VCell<Byte::Lo, 3>, 0x0583);
		ADD_STA(VCell<Byte::Hi, 2>, 0x0584);
		ADD_STA(VCell<Byte::Lo, 2>, 0x0585);
		ADD_STA(VCell<Byte::Hi, 1>, 0x0586);
		ADD_STA(VCell<Byte::Lo, 1>, 0x0587);


		template<Byte B> requires IsByte<B>
		struct BusBar : public IReg
		{
			uint8_t result;
		};
		//undef

		template<Byte B> requires IsByte<B>
		struct TsRef : public IReg
		{
			uint8_t result;
		};
		ADD_STA(TsRef<Byte::Hi>, 0x058C);

		template<size_t T>
		concept IsGpioChannel = T >= 1 and T <= 8;

		template<Byte B, size_t GpioChannel> requires IsGpioChannel<GpioChannel> and IsByte<B>
		struct Gpio : public IReg
		{
			uint8_t result;
		};
		ADD_STA(Gpio<Byte::Hi, 1>, 0x0590);
		ADD_STA(Gpio<Byte::Lo, 1>, 0x0591);
		ADD_STA(Gpio<Byte::Hi, 2>, 0x0592);
		ADD_STA(Gpio<Byte::Lo, 2>, 0x0593);
		ADD_STA(Gpio<Byte::Hi, 3>, 0x0594);
		ADD_STA(Gpio<Byte::Lo, 3>, 0x0595);
		ADD_STA(Gpio<Byte::Hi, 4>, 0x0596);
		ADD_STA(Gpio<Byte::Lo, 4>, 0x0597);
		ADD_STA(Gpio<Byte::Hi, 5>, 0x0598);
		ADD_STA(Gpio<Byte::Lo, 5>, 0x0599);
		ADD_STA(Gpio<Byte::Hi, 6>, 0x059a);
		ADD_STA(Gpio<Byte::Lo, 6>, 0x059b);
		ADD_STA(Gpio<Byte::Hi, 7>, 0x059c);
		ADD_STA(Gpio<Byte::Lo, 7>, 0x059d);
		ADD_STA(Gpio<Byte::Hi, 8>, 0x059e);
		ADD_STA(Gpio<Byte::Lo, 8>, 0x059f);

		template<Byte B> requires IsByte<B>
		struct DieTemp1 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct DieTemp2 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxCell : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxGpio : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxBat : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxRefl : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxVbg2 : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxAvaoRef : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxAvddRef : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxOVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxUVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxOTCBDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxUTDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct AuxVCBdoneDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct RefOVDac : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct DiagMain : public IReg
		{
			uint8_t result;
		};

		template<Byte B> requires IsByte<B>
		struct DiagHi : public IReg
		{
			uint8_t result;
		};

		template<size_t CellChannel> requires IsCellChannel<CellChannel>
		struct CBCellCtrl : public IReg
		{
			uint8_t undef;
		};

		struct VMBDoneThresh : public IReg
		{
			uint8_t undef;
		};

		struct MBTimerCtrl : public IReg
		{
			uint8_t undef;
		};

		struct CBDoneThresh : public IReg
		{
			uint8_t undef;
		};

		struct OTCBThresh : public IReg
		{
			uint8_t undef;
		};

		struct BalCtrl1 : public IReg
		{
			uint8_t undef;
		};

		struct BalCtrl2 : public IReg
		{
			uint8_t undef;
		};

		struct BalCtrl3 : public IReg
		{
			uint8_t undef;
		};

		struct CBComplete1 : public IReg
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

		struct CBComplete2 : public IReg
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

		struct BalTime : public IReg
		{
			uint8_t undef;
		};

		struct OVThresh : public IReg
		{
			uint8_t undef;
		};
	}

	class Bq79616
	{
	private:
		template<typename T> requires Utils::IsReg<T>
		uint32_t constexpr constructWrite()
		{
			constexpr uint32_t addr = Utils::sta<T>();
			//constexpr uint16_t count = 1; //serial write not supported
			constexpr uint32_t cmd = Cmd::WriteReg(addr);

			return cmd;
		}

		template<typename T> requires Utils::IsReg<T>
		uint32_t constexpr constructRead()
		{
			constexpr uint16_t addr = Utils::sta<T>();
			//constexpr uint16_t count = 1; //serial write not supported
			constexpr uint32_t cmd = Cmd::ReadReg(addr);

			return cmd;
		}

		SPI_HandleTypeDef *hspi;

		static inline constexpr size_t size = 6;
		std::array<uint32_t, size> out { 0 };
		std::array<uint32_t, size> in { 0 };

	public:
		explicit Ads131m04(SPI_HandleTypeDef *hspi) : hspi(hspi) { }
	};
}

#undef ADD_STA;

#endif /* INC_BQ79616_HPP_ */

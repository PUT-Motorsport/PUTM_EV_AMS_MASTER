#pragma once

namespace PUTM
{
    namespace Ads131m04
    {
        namespace Cmd
        {
            /*
             *  @brief  Cmd interface
             */
            struct ICmd
			{
				uint16_t cmd { 0 };
				uint32_t data[4] { 0 };
				uint16_t crc { 0 };
				uint16_t response { 0 };
			};

            /*
             *  @brief Null cmd definition
             */
            struct CmdNull : public ICmd
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

            /*
             *  @brief Reset cmd definition
             */
            struct CmdReset : public ICmd
			{
				CmdReset() { cmd = 0x0101; }
			};

            /*
             *  @brief Read reg cmd definition
             */
            template<typename T>
			struct CmdRReg : public ICmd
			{
				CmdRReg()
				{
					cmd = read_reg(Utils::address_of<T>());
				}
	
				T get_response()
				{
					return *((T*)&response);
				}
			};
        }
    }
}
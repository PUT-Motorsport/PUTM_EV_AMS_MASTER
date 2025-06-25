
namespace PUTM
{
    namespace Ads131m04
    {
        namespace Types
        {
            enum struct CrcType : uint16_t
            {
                CCITT,
                ANSI
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
                _128, _256, _512, _1024, _2048, _4096, _8192, _16256
            };

            enum struct PgaGain
            {
                _1, _2, _4, _8, _16, _32, _64, _128,
            };

            enum struct CdLen : uint16_t
            {
                _128, _256, _512, _768, _1280, _1792, _2560, _3584
            };

            enum struct CdNum : uint16_t
            {
                _1, _2, _4, _8, _16, _32, _64, _128,
            };

            enum struct GcDly : uint16_t
            {
                _2, _4, _8, _16, _32, _64, _128, _256, _512, _1024, _2048,
                _4096, _8192, _16384, _32768, _65536
            };

            enum struct DcBlock : uint16_t
            {
                DISABLE, _1_4, _1_8, _1_16, _1_32, _1_64, _1_128, _1_256,
                _1_512, _1_1024, _1_2048, _1_4096, _1_8192, _1_16384, 
                _1_32768, _1_65536
            };
            
            enum struct Mux : uint16_t
            {
                InPN,
                AdcShorted,
                DcTestP,
                DcTestN
            };

        }
    }
}
#pragma once

#include "cstring"
#include "string_view"

#include "main.h"
#include "stm32h5xx_hal_def.h"
#include <array>
#include <functional>
#include <queue>

namespace Utils
{
    /**
     *  @brief  This function is used to convert an integer to a uint8_t
     *  @param  `value` integer value to convert
     *  @return `value` casted to uint8_t 
     */
    uint8_t consteval operator ""ui8 (unsigned long long int value)
    {
        return static_cast<uint8_t>(value);
    }

    /**
     *  @brief  This function is used to convert an integer to a uint32_t
     *  @param  `value` integer value to convert
     *  @return `value` casted to uint8_t 
     */
    uint32_t consteval operator ""ui32 (unsigned long long int value)
    {
        return static_cast<uint32_t>(value);
    }

    /**
     *  @brief  This function is used to convert an time in min value to a uint32_t, ratio for min is 1:60 sec
     *  @param  `value` integer value to convert
     *  @return `value` casted to uint8_t 
     */
    uint32_t consteval operator ""min (unsigned long long int value)
    {
        return static_cast<uint32_t>(value * 60);
    }

    /**
     *  @brief  This function is used to convert an time in sec value to a uint32_t, ratio for sec is 1:1 sec
     *  @param  `value` integer value to convert
     *  @return `value` casted to uint8_t 
     */
    uint32_t consteval operator ""sec (unsigned long long int value)
    {
        return static_cast<uint32_t>(value);
    }


    /**
     *  @brief  This function is used to convert an integer representing seconds to a used time
     *          value, in this case its miliseconds (systems has 1000 ticks per second refresh rate)
     *  @param  `value` integer value to convert
     *  @return `value` in miliseconds
     */
    unsigned long long int consteval operator ""s (unsigned long long int value)
    {
        return value * 1000;
    }

    /**
     *  @brief  This function is used to convert an integer representing miliseconds to a used time
     *          value, in this case its miliseconds (systems has 1000 ticks per second refresh rate)
     *  @param  `value` integer value to convert
     *  @return `value` in miliseconds
     */
    unsigned long long int consteval operator ""ms (unsigned long long int value)
    {
        return value;
    }

    template<size_t ADDRESS_START, size_t ADDRESS_OFFSET = 0>
    struct IReg
    {
        static inline constexpr uint32_t ADDRESS = ADDRESS_START + ADDRESS_OFFSET;
    };

	template<typename T>
	concept IsAddressable = requires
    {
        { T::ADDRESS };
    };

    /**
    *   @brief  This function is wrap for structs which returns an address of
    *           of a object
    *   @tparam `ADDRESSABLE` an 'addressable' object, meaning it implements a
    *           static constexpr member `ADDRESS`
    *   @return Address of the object
    */
    template<IsAddressable ADDRESSABLE>
	uint32_t consteval address_of() // [[unused]] const ADDRESSABLE& a = { }
    {
        return ADDRESSABLE::ADDRESS;
    }

    /*
    *   @brief  This function copies the memory of the provided struct
    *           and returns it as a desired trivial class
    *   @tparam `TO` trivial class
    *   @tparam `FROM` trivial class
    *   @param  `s` data object
    *   @return Converted data
    */
    template<typename TO, typename FROM>
    TO convert_to(FROM s)
    {
        TO ret;
        std::memcpy(&ret, &s, sizeof(TO));
        return ret;
    }

    /**
    *   @brief  This function is used to throw compile errors in
    *           consteval function
    *   @note   This function should't be defined
    */
    void throw_consteval_failure(char const*);

    /**
     *  @brief  This function is used to run a function for each
     *           integer in range [I, N)
     *  @tparam `BEGIN` start index
     *  @tparam `END` end index
     *  @tparam `FUNCTION` function type
     *  @param  `f` function object
     *  @return void
     *  @note   This function is consteval, meaning it can be evaluated at compile time
     *          and it will run the function for each integer in range [I, N).
     *          The function object `f` should have a template member function `operator()<int I>()`
     *          which will be called for each integer in range [I, N).
     *  @example 
     *  static_for<0,3>([&]<int I>() {
     *  body of loop, where I is loop index
     *  });
     */
    template<int BEGIN, int END, class FUNCTION>
    constexpr void static_for(FUNCTION f) 
    {
        if constexpr (BEGIN < END) 
        {
            f.template operator()<BEGIN>();
            static_for<BEGIN + 1, END>(f);
        }
    }

    /**
     *  @brief  Count updates per second
     */
    struct UpdatesCounter
    {
        static constexpr size_t HISTORY_COUNT { 16 };
        uint32_t last_update { 0 };
        uint32_t last_values[HISTORY_COUNT] { 0 };
        uint32_t sum { 0 };
        size_t index { 0 };
        
        float update(uint32_t tick);
    };

    std::string_view get_error_name(HAL_StatusTypeDef error_code);
}
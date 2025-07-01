#pragma once

#include "cstring"

#include "main.h"

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

    /*
    *   @brief  This function is wrap for structs which returns an address of
    *           of a object
    *   @tparam `ADDRESSABLE` an 'addressable' object, meaning it implements a
    *           static constexpr member `ADDRESS`
    *   @param  `a` its used to call a function without specifying the template
    *           argument
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
     *  @tparam `I` start index
     *  @tparam `N` end index
     *  @tparam `F` function type
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
    consteval void static_for(FUNCTION f) 
    {
        if constexpr (BEGIN < END) 
        {
            f.template operator()<BEGIN>();
            static_for<BEGIN + 1, END>(f);
        }
    }
}
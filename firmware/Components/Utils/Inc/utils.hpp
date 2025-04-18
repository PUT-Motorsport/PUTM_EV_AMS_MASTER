#pragma once

#include "cstring"

#include "main.h"

namespace Utils
{
    uint8_t consteval operator ""ui8 (unsigned long long int value)
    {
        return static_cast<uint8_t>(value);
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

    /*
    *   @brief  This function is used to throw compile errors in
    *           consteval function
    *   @note   This function should't be defined
    */
    void throw_consteval_failure(char const*);
}
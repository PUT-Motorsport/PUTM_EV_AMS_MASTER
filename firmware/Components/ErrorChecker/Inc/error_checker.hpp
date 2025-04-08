#pragma once

#include "main.h"
#include "limits"

struct Error
{
    /* error name */
    const char *name { "" };
    /**
     *  @brief  Pointer to a function which should check condtion for the error to be
     *          raised
     *  @note   While this functions just checks if the return value is different from 0,
     *          it is advised to use unique error codes if one error type is checked 
     *          against multipe unique conditions
     *  @return The pointed function should return 0 when no errors where found, a value 
     *          bigger than 0 when an error was found
     */
    uint32_t (*condition)(void) { nullptr };
    
    /* PRIVATE */
    Error *next_error { nullptr };
    uint32_t id { std::numeric_limits<uint32_t>::max() };
};

template<size_t SIZE>
class ErrorLogger
{
public:

private:
};

class ErrorChecker
{
private:
    /**
     *  @brief  Add edges helper function
     *  @param  error Pointer to the next edge to be initialized by the state machine
     */
    void add_errors_helper(Error *error);
public:
    /**
     *  @brief  Variadic param function which adds multiple edges 
     *  @tparam ARGS Variadic template param it accepts all Error classes
     *  @param  errors variadic param for multiple edge init, pass all the defined edges
     *          to this function
     */
    template<typename ... ARGS>
    void add_errors(ARGS&&... errors)
    {
        (add_erros_helper(&errors), ...);
    }

private:
    /* Added errors behave like list Error checker remembers the first one */
    Error *next_error { nullptr };
    /* Last added error */
    Error *last_error { nullptr };
    uint32_t error_types_count { };
};
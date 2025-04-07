#pragma once

#include "main.h"

struct Error
{
    /* error name */
    const char *name { nullptr };
    /* condition to be passed for error to be raised */
    bool (*condition)() { nullptr };
    
    
    /* PRIVATE */
    Error *next_error { nullptr };
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
    /* added errors behave like list Error checker remembers the first one */
    Error *next_error { nullptr };
};
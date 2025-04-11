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
     *          bigger than 0 when an error was found. This value is used to identify the
     *          error sub type. It is advised to use unique error codes for each error sub 
     *          type.
     */
    uint32_t (*condition)(void) { nullptr };
    /**
     *  @brief  Pointer to a function which should parse the error code and return a 
     *          string with the error message.
     *  @note   This function is optional, it should be used to parse the error code and 
     *          return a string with the error message. It is advised to use unique error 
     *          codes for each error sub type.
     *  @param  code Error code to be parsed
     *  @return String with the error message, it should be a static string.
     */
    const char* (*parse)(uint32_t) { nullptr };
    /* PRIVATE */
    Error *next_error { nullptr };
};

/**
 *  @brief  Error checker class which is used to check for errors
 *  @note   This class is used to check for errors in the system and log them if they 
 *          are found. It uses a linked list to store the errors and a variadic template 
 *          function to add new errors to the list.
 */
class ErrorChecker
{
private:
    /**
     *  @brief  Add edges helper function
     *  @param  error Pointer to the next edge to be initialized by the error checker
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

    /**
     *  @brief  Function which checks for errors in the system
     *  @note   This function should be called periodically to check for errors in the 
     *          system. It will check all the errors in the list and log them if they 
     *          are found.
     */
    void check_errors(void);
private:
    /* Added errors behave like list Error checker remembers the first one */
    Error *next_error { nullptr };
    /* Last added error */
    Error *last_error { nullptr };
};
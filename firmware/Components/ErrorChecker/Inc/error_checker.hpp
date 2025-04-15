#pragma once

#include "main.h"
#include "limits"

namespace PUTM
{
    /**
     *  @brief  Error info class which is used to store the private error information
     */
    struct ErrorInfo
    {
    protected:
        friend struct ErrorChecker;
        /* Next error in the list */
        Error *next_error { nullptr };
        /* Next raised error in the list */
        Error *next_raised_error { nullptr };
        /* First time stamp at which the error was detected */
        uint32_t timestamp { 0 };
        /* */
        uint32_t accumulator { 0 };
    };

    struct Error : public ErrorInfo
    {
        /* Error name */
        const char *name { nullptr };
        /* If error persists for longer than the timeout value, an true error will be raised */
        uint32_t timeout { 0 }; 
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
    public:
        /**
         *  @brief  Function which checks for errors in the system
         *  @param  tick Time stamp of the system tick
         *  @note   This function should be called periodically to check for errors in the 
         *          system. It will check all the errors in the list and log them if they 
         *          are found.
         * @return  True if an error was found, false otherwise
         */
        bool check_errors(uint32_t tick);
    public:
        /**
         *  @brief  If an error was raised, this function will return the next error in the list
         *  @param  tick Time stamp of the system tick
         *  @note   This function should be called when `check_errors` returns true. It will return 
         *          the next error in the list which was raised. If no error was raised, it will return 
         *          nullptr.
         *  @return Pointer to the local copy of next error in the list which was raised, or nullptr 
         *          if no more errors were detected.
         */
        Error* get_next_error();
    public:
        const char* get_error_message(uint32_t code)
        {
            /* Check if error is valid */
            if(code == 0) return nullptr;
            /* Check if error is valid */
            if(next_error == nullptr) return nullptr;
            /* Check if error is valid */
            if(last_error == nullptr) return nullptr;
            /* Check if error is valid */
            if(last_error->parse == nullptr) return nullptr;
            /* Parse the error code and return the error message */
            return last_error->parse(code);
        }
    private:
        /* Added errors behave like list Error checker remembers the first one */
        Error *next_error { nullptr };
        /* Last added error */
        Error *last_error { nullptr };
        /* Next raised error */
        Error *next_raised_error { nullptr };
        /* Last raised error */
        Error *last_raised_error { nullptr };
        /* Error local copy */
        Error raised_error_copy { nullptr };
    };
}
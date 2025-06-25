#pragma once

#include "main.h"
#include "limits"

// TODO: change parse ptr to accept a string buffer and write to it instead of returning a string

namespace PUTM
{
    struct Error;
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
        /* Error added to raised list */
        bool added_to_raised_list { false };
        /* Last time stamp at which the error was detected */
        uint32_t timestamp { 0 };
        /* Time accumulator in [ms], its used to abstract the error timeout */
        int32_t accumulator { 0 };
    };

    /**
     *  @brief  Error class which is used to store the error information
     *  @note   The inherence from ErrorInfo allows this class to be trivially constructible
     *          while allowing for protected parameters.
     */
    struct Error : public ErrorInfo
    {
        /* Error name */
        const char *name { nullptr };
        /* If error persists for longer than the timeout value, an true error will be raised */
        int32_t timeout { 0 }; 
        /* Last error code */
        uint32_t last_code { 0 };
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

        const char* get_error_message()
        {
            /* Check if error is valid */
            if(this->last_code == 0) return "No error???";
            /* Check if error is valid */
            if(this->parse == nullptr) return "No parser???";
            /* Parse the error code and return the error message */
            return this->parse(this->last_code);
        }
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
            (add_errors_helper(&errors), ...);
        }
    public:
        /**
         *  @brief  Function which checks for errors in the system
         *  @param  tick Time stamp of the system tick
         *  @note   This function should be called periodically to check for errors in the 
         *          system. It will check all the errors in the list
         * @return  True if an error was found, false otherwise
         */
        bool check_errors(uint32_t tick);
    public:
        /**
         *  @brief  If an error was raised, this function will return the next error in the list
         *  @note   This function should be called when `check_errors` returns true. It will return 
         *          the next error in the list which was raised. If no error was raised, it will return 
         *          nullptr.
         *  @return Pointer to the local copy of next error in the list which was raised, or nullptr 
         *          if no more errors were detected.
         */
        Error* get_next_error();
    public:
#ifdef DEBUG_TEST_MODE_1
    public:
        /**
         *  @brief  Function which resets the error checker
         *  @note   This function should be called when the error checker is reset. It will reset
         */    
        void reset(void)
        {
            /* Reset the error checker */
            next_raised_error = nullptr;
            last_raised_error = nullptr;
            raised_error_copy = { };
            /* Iterate over all added erros */
            Error *error = next_error;
            while(error != nullptr)
            {
                /* Reset the error */
                error->added_to_raised_list = false;
                error->next_raised_error = nullptr;
                error->last_code = 0;
                error->accumulator = 0;
                error->timestamp = 0;
                error = error->next_error;
            }
        }
#endif
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
        Error raised_error_copy { };
    };
}
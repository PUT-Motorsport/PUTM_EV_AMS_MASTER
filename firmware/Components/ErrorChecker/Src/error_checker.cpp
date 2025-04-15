#include "main.h"

#include "error_checker.hpp"

using namespace PUTM;

void ErrorChecker::add_errors_helper(Error *error)
{
    // how?
    if(error == nullptr) return;
    // error was included earlier, omit it
    // if(error->id != std::numeric_limits<uint32_t>::max()) return;
    if(next_error == nullptr)
    {
        next_error = error;
        last_error = error;
    }
    else
    {
        last_error->next_error = error;
        last_error = error;
    }
}

bool ErrorChecker::check_errors(uint32_t tick)
{
    /* Update errors */
    Error *error = next_error;
    while(error != nullptr)
    {
        /* Check if error is valid */
        if(error->condition == nullptr) Error_Handler();
        /* Check if error is valid */
        if(error->name == nullptr) Error_Handler();
        
        /* Check for errors */
        uint32_t code = error->condition();
        if(error->timestamp == 0)
        {
            error->timestamp = tick;
            continue;
        }
        uint32_t time = tick - error->timestamp;
        if(code != 0)
        {
            if(time > error->timeout)
            {
                /* True error was found, log it */
                // log_error(error->name, code, error->parse(code));
                // reset the error
                error->count = 0;
            }
            else
            {
                /* Error was found, but it is not a true error yet */
                // log_error(error->name, code, error->parse(code));
                // increment the count
                error->count++;
            }
        }
        else
        {

        }
        error = error->next_error;
    }
    /* Check for true errors */
}

Error* ErrorChecker::get_next_error()
{
    /* Check if error is valid */
    if(next_raised_error == nullptr) return nullptr;

    raised_error_copy = *next_raised_error;

    next_raised_error = next_raised_error->next_raised_error;

    return &raised_error_copy;
}
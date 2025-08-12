#include "main.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"


#include "error_checker.hpp"

using namespace PUTM;

void Error::reset()
{
    this->accumulator = 0;
    this->last_code = 0;
    this->raised = false;
}

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
    /* Error was found */
    bool error_found = false;
    /* Update errors */
    Error *error = next_error;
    while(error != nullptr)
    {
        /* Check if error is valid */
        if(error->condition == nullptr) return true;
        /* Check if error is valid */
        if(error->name == nullptr) return true;
        
        /* Check for errors */
        uint32_t code = error->condition();

        /* *Time accumulator* */
        uint32_t time = tick - error->timestamp;
        if(code != 0)
        {
            error->accumulator += time;
            if(error->accumulator > error->timeout) error->accumulator = error->timeout;
        }
        else
        {
            error->accumulator -= time;
            if(error->accumulator < 0) error->accumulator = 0;
        }
        error->timestamp = tick;
        
        if(error->accumulator >= error->timeout) // and not error->raised
        {
            error->last_code = code;
            error->raised = true;
            error_found = true;
            if(error->callback != nullptr)
            {
                error->callback(error, code);
            }
        }
        else if (error->accumulator <= error->timeout / 2)
        {
            error->raised = false;
            error->last_code = 0;
        }
        error = error->next_error;
    }
    
    return error_found;
}


ErrorChecker::Iterator ErrorChecker::begin()
{
    Error* error = next_error;
    while(error != nullptr) 
    {
        if(error->raised) break;
        error = error->next_error; 
    }
    return Iterator(error);
}
ErrorChecker::Iterator ErrorChecker::end()
{
    return Iterator(nullptr);
}

ErrorChecker::Iterator& ErrorChecker::Iterator::operator++()
{
    Error* error = this->current;
    if(error != nullptr) error = error->next_error;
    while(error != nullptr) 
    {
        if(error->raised) break;
        error = error->next_error;
    }
    this->current = error;
    return *this;
} 

bool ErrorChecker::Iterator::operator==(const Iterator& other) const
{
    return current == other.current;
}

bool ErrorChecker::Iterator::operator!=(const Iterator& other) const
{
    return current != other.current;
}

Error& ErrorChecker::Iterator::operator*() const
{
    return *current;
}

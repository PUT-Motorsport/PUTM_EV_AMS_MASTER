#include "main.h"
#include "tx_api.h"
#include "cstdio"
#include "usart.h"

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
    /* Error was found */
    bool error_found = false;
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

        /* *Time accumulator* */
        uint32_t time = tick - error->timestamp;
        if(code != 0)
        {
            error->accumulator += time;
        }
        else
        {
            error->accumulator -= time;
            if(error->accumulator < 0) error->accumulator = 0;
        }
        error->timestamp = tick;
        
        
        // /* Print acu voltage */
        // char buffer[128] { 0 };
        // snprintf(buffer, sizeof(buffer), "Info: accumulator: %d\n", error->accumulator);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        // snprintf(buffer, sizeof(buffer), "Info: error name: %s\n", error->name);
        // HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        /* Check for timeout */
        if(error->accumulator > error->timeout)
        {
            error_found = true;
            if(next_raised_error == nullptr)
            {
                /* Begin listing */
                next_raised_error = error;
                last_raised_error = error;
            }
            //FIXME: This may not work as expected, if the error is not added to the list, it will be added again
            /* Prevent looping */
            else if(error->added_to_raised_list == false)
            {
                /* Add to list */
                last_raised_error->next_raised_error = error;
                last_raised_error = error;
            }
            error->added_to_raised_list = true;
            error->accumulator = 0;
            error->last_code = code;
        }
        error = error->next_error;
    }
    
    return error_found;
}

Error* ErrorChecker::get_next_error()
{
    /* Check if error is valid */
    if(next_raised_error == nullptr) return nullptr;

    /* Make shallow copy */
    //FIXME: Make shallow copy of the error without the pointers inside, maybe use a shared_ptr or a unique_ptr
    raised_error_copy = *next_raised_error;

    /* End list */
    if(next_raised_error == last_raised_error) 
    {
        next_raised_error = nullptr;    
        last_raised_error = nullptr;
    }
    /* Iterate list */
    else
    {
        Error *buffer = next_raised_error->next_raised_error;
        next_raised_error->next_raised_error = nullptr;
        next_raised_error->added_to_raised_list = false;
        next_raised_error = buffer;
    }
    
    return &raised_error_copy;
}

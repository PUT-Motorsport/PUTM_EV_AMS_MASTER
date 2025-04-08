#include "error_checker.hpp"

void ErrorChecker::add_errors_helper(Error *error)
{
    // how?
    if(error == nullptr) return;
    // error was included earlier, omit it
    if(error->id != std::numeric_limits<uint32_t>::max()) return;
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
    
    error->id = error_types_count;
    error_types_count++;
}
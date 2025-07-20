#pragma once

#include "main.h"
#include "cstring"

namespace PUTM
{
    struct ErrorLog
    {
        /* Error name */
        const char *name { "" };
        /* Error code */
        uint32_t code { 0 };
        /* Error message */
        const char *message { "" };
        /* Time stamp */
        uint32_t timestamp { 0 };
    }

    template<size_t SIZE>
    class ErrorLogger
    {
    public:
        /**
         *  @brief  Function which logs an error
         *  @param  name Name of the error
         *  @param  code Error code
         *  @param  message Error message
         *  @note   This function will log the error in the error logger buffer. If the buffer
         *          is full, the error will not be logged and the overflow flag will be set to
         *          true.
         */    
        void log_unique_error(const char *name, uint32_t code, const char *message)
        {
            if (index < size)
            {
                for(size_t i = 0; i < index; i++)
                    if(strcmp(buffer[i].name, name) == 0 and buffer[i].code == code)
                        return;
                buffer[index].name = name;
                buffer[index].code = code;
                buffer[index].message = message;
                buffer[index].timestamp = HAL_GetTick();
                index++;
            }
            else
            {
                overflow = true;
            }
        }
    private:
        /* Error logger buffer */
        ErrorLog buffer[SIZE] { 0 };
        /* Error logger buffer index */
        size_t index { 0 };
        /* Error logger buffer size */
        size_t size { SIZE };
        /* Error logger buffer overflow flag */
        bool overflow { false };
    };
}
#pragma once

#include "main.h"

#include "cstring"

#include "string_view"
#include "format"
#include "cstdint"

/*
in the future maybe make the buffer circular to store an arbitrary number of messages, but for now just store a fixed number of messages and discard the rest
*/

namespace PUTM
{
    /**
     * @brief Fixed-size in-memory logger for timestamped event and error messages.
     *
     * @tparam BUFFER_SIZE Total size of the internal buffer, in bytes.
     * @tparam MESSAGE_INDEX_TYPE Unsigned integer type used to store message end offsets.
     *
     * @details
     * The logger stores formatted log messages at the beginning of an internal buffer and
     * stores message end indices at the end of the same buffer. This allows individual
     * messages to be retrieved later without copying the message text.
     *
     * Each logged message is formatted as:
     *
     * @code
     * 000000000 ERR: message
     * 000000000 EVT: message
     * @endcode
     *
     * where the numeric prefix is the supplied tick value padded to 9 digits.
     *
     * @note
     * The logger does not dynamically allocate memory.
     *
     * @note
     * If there is not enough space for a message and its index metadata, the message is
     * silently discarded.
     *
     * @warning
     * Returned std::string_view objects refer directly to the internal buffer. They become
     * stale if the logger is cleared.
     *
     * @warning
     * MESSAGE_INDEX_TYPE must be large enough to represent all valid offsets inside the
     * buffer.
     */
    template<size_t BUFFER_SIZE, typename MESSAGE_INDEX_TYPE = uint16_t> // requires BUFFER_SIZE to be less than 65536 for MESSAGE_INDEX_TYPE = uint16_t
    class Logger
    {
    private:
        using MSG_I_TYPE = MESSAGE_INDEX_TYPE;
    protected:
        char __buffer[BUFFER_SIZE];
        MSG_I_TYPE __free_space = BUFFER_SIZE;
        MSG_I_TYPE __index { 0 };
        MSG_I_TYPE __message_count { 0 };
    protected:
        friend class Iterator;
    public:
        /**
        *  @brief  This function logs a message to the logger
        *  @param  `message` message to log
        *  @note   The message is expected to be terminated with a newline character.
        *          If the message is too long, it will be truncated to fit in the buffer
        *          and the rest of the message will be discarded.
        *          The message is prefixed with "ERR:" to indicate that it is an error message.
        *          The function will not log the message if there is not enough space in
        *          the buffer to fit the message.
        */
        void log_error(std::string_view message, uint32_t tick = 0)
        {
            MSG_I_TYPE *__ptr;
            
            /* include null-termination */
            if(__index + message.size() + 17 + sizeof(MSG_I_TYPE) >= __free_space) return;
            if(message.size() == 0) return; // No message to log
            std::format_to_n(__buffer + __index, __free_space - __index, "{:09d} ERR: {} \n", tick, message);
            __index += message.size() + 17;
            __ptr = (MSG_I_TYPE*)(__buffer + __free_space - sizeof(MSG_I_TYPE));
            /* store the message 'end index' at the heap beginning at the end of the buffer */
            *__ptr = __index;
            __free_space -= sizeof(MSG_I_TYPE);
            __message_count++;
        }
    public:
        /**
         *  @brief  This function logs a message to the logger
         *  @param  `message` message to log
         *  @note   The message is expected to be terminated with a newline character.
         *          If the message is too long, it will not be logged to the buffer
         *          and the rest of the message will be discarded.
         *          The message is prefixed with "EVT:" to indicate that it is an event message.
         *          The function will not log the message if there is not enough space in
         *          the buffer to fit the message.
         */
        void log_event(std::string_view message, uint32_t tick = 0)
        {
            MSG_I_TYPE *__ptr;
            
            /* include null-termination */
            if(__index + message.size() + 17 + sizeof(MSG_I_TYPE) >= __free_space) return;
            if(message.size() == 0) return; // No message to log
            std::format_to_n(__buffer + __index, __free_space - __index, "{:09d} EVT: {} \n", tick, message);
            __index += message.size() + 17;
            __ptr = (MSG_I_TYPE*)(__buffer + __free_space - sizeof(MSG_I_TYPE));
            /* store the message 'end index' at the heap beginning at the end of the buffer */
            *__ptr = __index;
            __free_space -= sizeof(MSG_I_TYPE);
            __message_count++;
        }
    public:
        /**
        *  @brief  This function clears the logger
        *  @note   This function will reset the index, message count and size of the logger.
        *          It will not clear the buffer, so the messages will still be there.
        */
        void clear()
        {
            __index = 0;
            __message_count = 0;
            __free_space = BUFFER_SIZE;
        }
    public:
        std::string_view get_buffer() const
        {
            return std::string_view(__buffer, BUFFER_SIZE);
        }
    public:
        std::string_view get_buffer_and_clear()
        {
            std::string_view view;
            
            view = std::string_view(__buffer, __index);
            this->clear();
            return view;
        }
    protected:
        /**
        *  @brief  This function returns the range of the message at the given index
        *  @param  `message_index` index of the message to return
        *  @note   This function will return an empty string if the index is out of bounds
        *  @return std::pair<MSG_I_TYPE, MSG_I_TYPE> of the begin and end of the message at the given index
        */
        std::pair<MSG_I_TYPE, MSG_I_TYPE> get_range(size_t message_index) const
        {
            // MSG_I_TYPE *__ptr_begin { nullptr };
            MSG_I_TYPE *__ptr_end { nullptr };
            size_t __begin { 0 };
            size_t __end { 0 };

            if(__message_count == 0) return { 0, 0 }; // No messages
            if(message_index >= __message_count) return { 0, 0 }; // Out of bounds__buffer
            
            __ptr_end = (MSG_I_TYPE*)(__buffer + BUFFER_SIZE - sizeof(MSG_I_TYPE) * (message_index + 1));
            __end = *__ptr_end;
            if(message_index != 0)
            {
                MSG_I_TYPE *__ptr_begin = __ptr_end + 1;
                __begin = *__ptr_begin;
            }
            
            return { __begin, __end };
        }
    public:
        /**
        *  @brief  This function returns a message at the given index
        *  @param  `message_index` index of the message to return
        *  @note   This function will return an empty string if the index is out of bounds
        *  @return std::string_view of the message at the given index
        *  @note   The message is returned as a string_view, so it is not copied, but rather a view of the original buffer.
        *          The message is expected to be terminated with a newline character.
        *          If the index is out of bounds, an empty string_view is returned.
        *          The messages are separated by newline characters, so the function will return
        *          the message at the given index, where index 0 is the first message.
        */
        std::string_view get_message(size_t message_index) const
        {
            // MSG_I_TYPE *__ptr_begin { nullptr };
            // MSG_I_TYPE *__ptr_end { nullptr };
            // size_t __begin { 0 };
            // size_t __end { 0 };
            
            // if(message_index >= __message_count) return std::string_view(""); // Out of bounds
            
            // __ptr_end = (MSG_I_TYPE*)(__buffer + __free_space - sizeof(MSG_I_TYPE) * __message_count)
            // __end = *__ptr_end;
            // if(message_count != 1)
            // {
            //     MSG_I_TYPE *__ptr_begin = (MSG_I_TYPE*)(__buffer + __free_space - sizeof(MSG_I_TYPE) * (__message_count - 1))
            //     __begin = *__ptr_begin;
            // }
            
            if(__message_count == 0) return std::string_view(); // No messages
            if(message_index >= __message_count) return std::string_view(); // Out of bounds

            auto [__begin, __end] = this->get_range(message_index);
            
            return std::string_view(__buffer + __begin, __end - __begin);
        }
    public:
        bool is_full() const
        {
            return __index >= __free_space;
        }
    public:
        bool is_empty() const
        {
            return __message_count == 0;
        }
    public:
        size_t size() const
        {
            return __index;
        }
    public:
        size_t free_space() const
        {
            return __free_space - __index;
        }
    public:
        size_t message_count() const
        {
            return __message_count;
        }
    public:
        class Iterator
        {
        private:
            const Logger<BUFFER_SIZE> *__logger;
            size_t __index;
        public:
            Iterator(const Logger<BUFFER_SIZE> *logger, size_t index) : __logger(logger), __index(index) { }
            std::string_view operator*() const
            {
                return __logger->get_message(this->__index);
            }  
            Iterator& operator++()
            {
                this->__index++;

                return *this;
            }
            bool operator!=(const Iterator& other) const
            {
                return this->__index != other.__index;
            }
            bool operator==(const Iterator& other) const
            {
                return this->__index == other.__index;
            }
        };
    public:
        using iterator = Iterator;
        using const_iterator = Iterator;
    public:
        /**
        *  @brief  Returns an iterator to the beginning of the logger
        *  @return Iterator to the beginning of the logger
        */ 
        Iterator begin() const
        {
            return Iterator(this, 0);
        }
    public:
        /**
        *  @brief  Returns an iterator to the end of the logger
        *  @return Iterator to the end of the logger
        */
        Iterator end() const
        {
            return Iterator(this, this->__message_count);
        }
    };
}
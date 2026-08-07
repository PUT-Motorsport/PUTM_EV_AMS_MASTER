#pragma once

#include "main.h"

#include <cstddef>
#include <functional>

namespace PUTM
{
    struct EventCallback;

    class Event
    {
    private:
        // bool __raised { false };
        EventCallback *__first { nullptr };
    public:
        Event() = default;
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;
    private:
        struct AddEventCallbackHelper
        {   
            Event *__event;
            EventCallback *__last_added { nullptr };
            bool operator()(EventCallback *event_callback)
            {
                if(__event->__first == nullptr) 
                {
                    __event->__first = event_callback;
                    __last_added = event_callback
                    return true;
                }

                if(__last_added->__next == nullptr)
                {
                    __last_added->__next = event_callback;
                    __last_added = event_callback;
                    return true;
                }

                return false;
            }
        };
        AddEventCallbackHelper add_event_callback { this };
    public:
        void raise()
        {
            EventCallback *iterator {__first};
            while(iterator != nullptr)
            {
                iterator->operator()();
                iterator = iterator->__next;
            }
        }
    public:
        bool add(EventCallback *event_callback)
        {
            add_event_callback(event_callback);
        }
    public:
        /**
         *  @brief  Variadic param function which adds multiple event callbacks 
         *  @tparam ARGS Variadic template param it accepts all event callback classes
         *  @param  event_callbacks variadic param for multiple event callback init, pass all the defined callbacks
         *          to this function
         */
        template<typename ... ARGS>
        void add_multipule(ARGS&&... event_callbacks)
        {
            (add_event_callback(&event_callbacks), ...);
        }
    };

    class EventCallback // true to use light functions its more so a wrapper to notifiy other threads
    {
    public:
        friend Event;
    protected:
        std::function<void()> __callback;
        EventCallback *__next { nullptr };
    public:
        explicit constexpr EventCallback(std::function<void()> callback) : __callback(callback) { };
    protected:
        void operator()()
        {
            __callback()
        }
    };
}
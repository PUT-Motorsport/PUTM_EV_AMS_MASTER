#pragma once

#include "main.h"

namespace PUTM
{
    struct StateEdge;
    
    struct State
    {
        /* state name, it needs to be unique */    
        const char *name { "" };
        /* what to do on enter, can be nullptr */
        void (*on_enter)(void) { nullptr };
        /* what to do on update */
        void (*on_update)(void) { nullptr };
        /* what to do on exit, can be nullptr */
        void (*on_exit)(void) { nullptr };
    
        /* PRIVATE */
        /* next edge, behaves like a list not meant for user */
        StateEdge* edge { nullptr };
    };
    
    struct StateEdge
    {
        /* edge name best practice is to name it for example "idle -> on" */
        const char *name = { "" };
        /* if condition returns true machine will pass onto next state */
        bool (*condition)(void) { nullptr };
        /* prev state */
        State* prev_state { nullptr };
        /* next state */
        State* next_state { nullptr };
    
        /* PRIVATE */
        /* next edge, behaves like a list not meant for user */
        StateEdge* next_edge { nullptr };
    };
    
    class StateMachine
    {
    public:
        State* current_state { nullptr };
    public:
        /**
         *  @brief  Udate state machine, call on_update fun
         */
        void update();
    public:
        /**
         *  @brief  Start state machine from state
         *  @param  state Pointer to the first state from which the machine should start
         */
        void start(State* state);
    private:
        /**
         *  @brief  Add edges helper function
         *  @param  edge Pointer to the next edge to be initialized by the state machine
         */
        void add_edges_helper(StateEdge *edge);
    public:
        /**
         *  @brief  Variadic param function which adds multiple edges 
         *  @tparam ARGS Variadic template param it accepts all StateEdge classes
         *  @param  edges variadic param for multiple edge init, pass all the defined edges
         *          to this function
         */
        template<typename ... ARGS>
        void add_edges(ARGS&&... edges)
        {
            (add_edges_helper(&edges), ...);
        }
    };
}
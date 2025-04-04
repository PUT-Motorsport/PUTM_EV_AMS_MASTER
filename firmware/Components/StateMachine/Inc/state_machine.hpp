#pragma once

#include "main.h"

struct StateEdge;

struct State
{
    /* state name, it needs to be unique */    
    // constexpr char* name { "" };
    /* what to do on enter, can be nullptr */
    void (*on_enter)(void) { nullptr };
    /* what to do on exit, can be nullptr */
    void (*on_exit)(void) { nullptr };

    StateEdge* connection { nullptr };

    void add_connection(StateEdge* state_edge);
}

struct StateEdge
{
    /* if condition returns true machine will pass onto next state */
    bool (*condition)(void) { nullptr };
    /* next state */
    State* next_state { nullptr };
    /* next edge, behaves like a list*/
    StateEdge* next_connection { nullptr };
}

struct StateMachine
{
    State* current_state;
    void update();
}
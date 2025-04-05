#include "state_machine.hpp"

using namespace PUTM;

void StateMachine::update()
{
    
    if(current_state == nullptr) return;

    if(current_state->on_update != nullptr)
    {
        current_state->on_update();
    }

    StateEdge *edge = current_state->edge;
    while(edge != nullptr)
    {
        if(edge->condition == nullptr) break;
        if(edge->condition())
        {
            if(edge->next_state == nullptr) break;
            if(current_state->on_exit != nullptr)
            {
                current_state->on_exit();
            }
            current_state = edge->next_state;
            if(current_state->on_enter != nullptr)
            {
                current_state->on_enter();
            }
            break;
        }

        edge = edge->next_edge;
    }
    return;
}

void StateMachine::start(State* state)
{
    current_state = state;
    if(current_state->on_enter != nullptr)
    {
        current_state->on_enter();
    }
}

void StateMachine::add_edges_helper(StateEdge *edge)
{
    StateEdge **edge_container;

    if(edge == nullptr) return;
    if(edge->prev_state == nullptr) return;

    edge_container = &edge->prev_state->edge;
    while(*edge_container != nullptr) 
    {
        if(*edge_container == edge) return;
        edge_container = &((*edge_container)->next_edge);
    }
    *edge_container = edge;
}
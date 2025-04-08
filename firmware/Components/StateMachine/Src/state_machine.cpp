#include "state_machine.hpp"

using namespace PUTM;

void StateMachine::update()
{
    
    if(current_state == nullptr) return;

    if(current_state->on_update != nullptr)
    {
        current_state->on_update();
    }

    StateEdge *edge = current_state->first_edge;
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
    if(edge == nullptr) return;
    if(edge->prev_state == nullptr) return;
    if(edge->prev_state->first_edge == nullptr)
    {
        edge->prev_state->first_edge = edge;
        edge->prev_state->last_edge = edge;
        return;
    }
    // FIXME: yes, user can brake this code if he really wants to
    // maybe fix it some time
    edge->prev_state->last_edge->next_edge = edge;
    edge->prev_state->last_edge = edge;
}
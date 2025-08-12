#include "state_machine.hpp"

using namespace PUTM;

void StateMachine::update()
{
    if(current_state == nullptr) return;

    if(current_state->on_update != nullptr)
    {
        current_state->on_update();
    }

    /* handle generic edges */
    if(first_generic_edge != nullptr)
    {
        StateEdge *generic_edge = first_generic_edge;
        while(generic_edge != nullptr)
        {
            if(generic_edge->condition != nullptr and generic_edge->next_state != nullptr and generic_edge->next_state != current_state)
            {
                if(generic_edge->condition())
                {
                    if(current_state->on_exit != nullptr)
                    {
                        current_state->on_exit();
                    }
                    current_state = generic_edge->next_state;
                    if(current_state->on_enter != nullptr)
                    {
                        current_state->on_enter();
                    }
                    break;
                }
            }

            generic_edge = generic_edge->next_edge;
        }
    }

    /* handle normal edges*/
    StateEdge *edge = current_state->first_edge;
    while(edge != nullptr)
    {
        if(edge->condition != nullptr and edge->next_state != nullptr and edge->next_state != current_state)
        {
            if(edge->condition())
            {
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
    /* handle generic edges */
    if(edge->prev_state == &this->any_state)
    {
        if(this->first_generic_edge == nullptr)
        {
            this->first_generic_edge = edge;
            this->last_generic_edge = edge;
        }
        else 
        {
            this->last_generic_edge->next_edge = edge;
        }
        return;
    }
    /* handle normal edges */
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

void StateMachine::set_current_state(State* state)
{
    current_state = state;
} 

State* StateMachine::get_current_state() const
{
    return current_state;
}

std::string_view StateMachine::get_current_state_name() const
{
    if(current_state == nullptr) return { "IN LIMBO" };
    return { current_state->name };
}
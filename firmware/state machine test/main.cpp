#include <iostream>

struct StateEdge;

struct State
{
    /* state name, it needs to be unique */    
    char const *name { "" };
    /* what to do on enter, can be nullptr */
    void (*on_enter)(void) { nullptr };
    /* what to do on update */
    void (*on_update)(void) { nullptr };
    /* what to do on exit, can be nullptr */
    void (*on_exit)(void) { nullptr };

    /* next edge, behaves like a list not meant for user */
    StateEdge* edge { nullptr };
};

struct StateEdge
{
    /* edge name best practice is to name it for example "idle -> on" */
    char const *name = { "" };
    /* if condition returns true machine will pass onto next state */
    bool (*condition)(void) { nullptr };
    /* prev state */
    State* prev_state { nullptr };
    /* next state */
    State* next_state { nullptr };
    /* next edge, behaves like a list not meant for user */
    StateEdge* next_edge { nullptr };
};

struct StateMachine
{
    State* current_state { nullptr };
    void update()
    {
        if(current_state == nullptr) return;
        edge 
    }

    void _add_edges_helper(StateEdge *edge)
    {
        StateEdge **edge_container;

        edge_container = &edge->prev_state->edge;
        while(*edge_container != nullptr) edge_container = &((*edge_container)->next_edge);
        *edge_container = edge;
    }

    template<typename ... ARGS>
    void add_edges(ARGS&&... args)
    {
        (_add_edges_helper(&args), ...);
    }
};

State idle
{
    .name = "idle",
    .on_enter = [](){ std::cout << "entering idle"; },
    .on_exit = [](){ std::cout << "leaving idle"; },
};

State on
{
    .name = "on",
    .on_enter = [](){ std::cout << "entering on"; },
    .on_exit = [](){ std::cout << "leaving on"; },
};

State off
{
    .name = "off",
    .on_enter = [](){ std::cout << "entering off"; },
    .on_exit = [](){ std::cout << "leaving off"; },
};

size_t sth = 0;

StateEdge idle_to_on
{
    .name = "idle -> on",
    .condition = [](){ return sth == 1; },
    .prev_state = &idle,
    .next_state = &on,
};

StateEdge on_to_idle
{
    .name = "on -> idle",
    .condition = [](){ return sth == 2; },
    .prev_state = &on,
    .next_state = &idle,
};

StateEdge idle_to_off
{
    .name = "idle -> off",
    .condition = [](){ return sth == 3; },
    .prev_state = &idle,
    .next_state = &off,
};

StateMachine state_machine;

int main()
{

    std::cout<<"Hello World";

    return 0;
}
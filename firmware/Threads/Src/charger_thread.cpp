#include <cstddef>
extern "C"
{
#include "main.h"
}
#include "cstring"
#include "cstdio"
#include "usart.h"
#include "algorithm"
#include "fdcan.h"

#include "ArduinoJson.h"
// #include "can_interface.hpp"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "charger.hpp"
#include "wrapper/fdcan.hpp"
#include "wrapper/uart.hpp"
#include "state_machine.hpp"
#include "com/charger_state_machine.hpp"
#include "logger.hpp"
#include "string_view"

using namespace PUTM;

// using namespace PUTM_CAN;

extern StateMachine charger_state_machine;

VOID charger_thread_entry(__unused ULONG thread_input)
{
    start_can(&hfdcan1);

    init_charger_state_machine(&charger_state_machine);

    while(true)
    {
        charger_state_machine.update();
        tx_thread_sleep(50);
    }
}
/*
* tim_wrap.hpp
*
*  Created on: Feb 8, 2025
*      Author: Piotr lesicki
*/

#ifndef INC_UART_WRAP_HPP_
#define INC_UART_WRAP_HPP_

#include "main.h"
#include "usart.h"

enum struct UartStatus : uint8_t
{
    Ok,
    __NOT_USED__,
    Timeout, // Tx timeout?
    Error
};

/**
  * @brief HAL UART State definition
  * @note  HAL UART State value is a combination of 2 different substates:
  *        gState and RxState (see @ref UART_State_Definition).
  *        - gState contains UART state information related to global Handle management
  *          and also information related to Tx operations.
  *          gState value coding follow below described bitmap :
  *          b7-b6  Error information
  *             00 : No Error
  *             01 : (Not Used)
  *             10 : Timeout
  *             11 : Error
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral initialized. HAL UART Init function already called)
  *          b4-b3  (not used)
  *             xx : Should be set to 00
  *          b2     Intrinsic process state
  *             0  : Ready
  *             1  : Busy (Peripheral busy with some configuration or internal operations)
  *          b1     (not used)
  *             x  : Should be set to 0
  *          b0     Tx state
  *             0  : Ready (no Tx operation ongoing)
  *             1  : Busy (Tx operation ongoing)
  *        - RxState contains information related to Rx operations.
  *          RxState value coding follow below described bitmap :
  *          b7-b6  (not used)
  *             xx : Should be set to 00
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral initialized)
  *          b4-b2  (not used)
  *            xxx : Should be set to 000
  *          b1     Rx state
  *             0  : Ready (no Rx operation ongoing)
  *             1  : Busy (Rx operation ongoing)
  *          b0     (not used)
  *             x  : Should be set to 0.
  */
struct __packed UartState
{
public:
    explicit UartState(uint32_t g_state) 
    { 
        tx_busy = (uint8_t)((g_state >> 0) & 0x01);
        rx_busy = (uint8_t)((g_state >> 1) & 0x01);
        uart_busy = (uint8_t)((g_state >> 2) & 0x01);
        init_done = (uint8_t)((g_state >> 5) & 0x01);
        status = (UartStatus)((g_state >> 6) & 0x03);
    }
    bool tx_busy : 1 { 0 };
    bool rx_busy : 1 { 0 };
    bool uart_busy : 1 { 0 };
private:
    uint8_t reserved : 2 { 0 };
public:
    bool init_done : 1 { 0 };
    UartStatus status : 2 { 0 };
};

#endif /* INC_UART_WRAP_HPP_ */
#ifndef __TASKS_HPP__
#define __TASKS_HPP__

#include "tx_api.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *  @brief  Initialize stuf
 */
VOID init();

/**
 *  @brief  Main thread, manages all other threads states, internal states etc.
 */
VOID main_thread_entry(ULONG thread_input);

/**
 *  @brief  Test thread used only for tesing experimental stuff
 */
VOID test_thread_entry(ULONG thread_input);

/**
 *  @brief  BQ796XX thread manages ic communication and writes to global data storage
 */
VOID bq796xx_thread_entry(ULONG thread_input);

/**
 *  @brief  ADS131M04 thread manages ic communication and writes to global data storage
 */
VOID ads131m04_thread_entry(ULONG thread_input);

/**
 *  @brief  USB TX thread, usbx framework works based on a polling scheme this thread is 
 *          only resposible for sending packets from a private char buffer
 */
VOID usb_tx_thread_entry(ULONG thread_input);

/**
 *  @brief  USB TX thread, usbx framework works based on a polling scheme this thread is 
 *          only resposible for receving packets and writing them to a private char buffer
 */
VOID usb_rx_thread_entry(ULONG thread_input);

/**
 *  @brief  USB general communication task it is responisible for serializing data and sending
 *          it to host
 */
VOID usb_com_thread_entry(ULONG thread_input);

/**
 *  @brief  Car can thread, it is used to send data to car
*/
VOID car_can_thread_entry(ULONG thread_input);

/**
 *  @brief  Charger can thread, it is used to send data to charger
 */
VOID charger_can_thread_entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TASKS_HPP */
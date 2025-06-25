#include "main.h"
#include "tx_api.h"
#include "ArduinoJson.h"

#include "threads.hpp"

TX_THREAD test_thread;
static constexpr size_t test_thread_pool_size { 1024U };
uint8_t test_thread_pool[test_thread_pool_size];

TX_THREAD bq796xx_thread;
static constexpr size_t bq796xx_thread_pool_size { 1024U };
uint8_t bq796xx_thread_pool[bq796xx_thread_pool_size];

TX_THREAD ads131m04_thread;
static constexpr size_t ads131m04_thread_pool_size { 1024U };
uint8_t ads131m04_thread_pool[ads131m04_thread_pool_size];

TX_THREAD main_thread;
static constexpr size_t main_thread_pool_size { 2048U };
uint8_t main_thread_pool[main_thread_pool_size];

TX_THREAD usb_com_thread;
static constexpr size_t usb_com_thread_pool_size { 1024U };
uint8_t usb_com_thread_pool[usb_com_thread_pool_size];

TX_THREAD usb_tx_thread;
static constexpr size_t usb_tx_thread_pool_size { 1024U };
uint8_t usb_tx_thread_pool[usb_tx_thread_pool_size];

TX_THREAD usb_rx_thread;
static constexpr size_t usb_rx_thread_pool_size { 1024U };
uint8_t usb_rx_thread_pool[usb_rx_thread_pool_size];

TX_THREAD car_can_thread;
static constexpr size_t car_can_thread_pool_size { 1024U };
uint8_t car_can_thread_pool[car_can_thread_pool_size];

TX_THREAD charger_can_thread;
static constexpr size_t charger_can_thread_pool_size { 1024U };
uint8_t charger_can_thread_pool[charger_can_thread_pool_size];

TX_SEMAPHORE data_semaphore;

VOID init()
{
#ifdef DEBUG_TEST_MODE_1
    /**
     *  @note This thread is used only for testing purposes, it is not used in the final application
     *        it is used to test the system and to check if the system is working properly. Define the
     *        TEST_MODE_1 macro to enable this thread.
     */
    tx_thread_create(test_thread, (CHAR*)"Test thread", test_thread_entry, 0U, test_thread_pool, test_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
#else    
    tx_thread_create(&bq796xx_thread, (CHAR*)"BQ796XX thread", bq796xx_thread_entry, 0U, bq796xx_thread_pool, bq796xx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&ads131m04_thread, (CHAR*)"ADS131M04 thread", ads131m04_thread_entry, 0U, ads131m04_thread_pool, ads131m04_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
#endif /* TEST_MODE_1 */
    tx_thread_create(&main_thread, (CHAR*)"Main thread", main_thread_entry, 0U, main_thread_pool, main_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_com_thread, (CHAR*)"-USB COM INIT thread-", usb_com_thread_entry, 0U, usb_com_thread_pool, usb_com_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_tx_thread, (CHAR*)"USB TX thread", usb_tx_thread_entry, 0U, usb_tx_thread_pool, usb_tx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_rx_thread, (CHAR*)"USB RX thread", usb_rx_thread_entry, 0U, usb_rx_thread_pool, usb_rx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&car_can_thread, (CHAR*)"Other COM thread", car_can_thread_entry, 0U, car_can_thread_pool, car_can_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&charger_can_thread, (CHAR*)"Charger COM thread", charger_can_thread_entry, 0U, charger_can_thread_pool, charger_can_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_semaphore_create(&data_semaphore, (CHAR*)"Data semaphore", 0U);
    
    //tx_thread_suspend(&charger_can_thread);
}
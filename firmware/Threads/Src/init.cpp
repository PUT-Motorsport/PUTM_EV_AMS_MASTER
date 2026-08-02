#include "main.h"
#include "tx_api.h"
#include "ArduinoJson.h"

#include "threads.hpp"
#include "config.hpp"

using namespace PUTM;

TX_THREAD test_thread;
uint8_t test_thread_pool[Config::TEST_THREAD_POOL_SIZE];

TX_THREAD bq796xx_thread;
uint8_t bq796xx_thread_pool[Config::BQ796xx_THREAD_POOL_SIZE];

TX_THREAD ads131m04_thread;
uint8_t ads131m04_thread_pool[Config::ADS131m04_THREAD_POOL_SIZE];

TX_THREAD main_thread;
uint8_t main_thread_pool[Config::MAIN_THREAD_POOL_SIZE];

TX_THREAD usb_com_thread;
uint8_t usb_com_thread_pool[Config::USB_COM_THREAD_POOL_SIZE];

// TX_THREAD usb_tx_thread;
// static constexpr size_t usb_tx_thread_pool_size { 1024U };
// uint8_t usb_tx_thread_pool[usb_tx_thread_pool_size];

// TX_THREAD usb_rx_thread;
// static constexpr size_t usb_rx_thread_pool_size { 1024U };
// uint8_t usb_rx_thread_pool[usb_rx_thread_pool_size];

TX_THREAD car_can_thread;
uint8_t car_can_thread_pool[Config::CAR_CAN_THREAD_POOL_SIZE];

TX_THREAD charger_can_thread;
uint8_t charger_can_thread_pool[Config::CHARGER_CAN_THREAD_POOL_SIZE];

TX_SEMAPHORE data_semaphore;

TX_TIMER soc_update_timer;

VOID init()
{
// #ifdef DEBUG_TEST_MODE_1
    /**
     *  @note This thread is used only for testing purposes, it is not used in the final application
     *        it is used to test the system and to check if the system is working properly. Define the
     *        TEST_MODE_1 macro to enable this thread.
     */
    tx_thread_create(&test_thread, (CHAR*)"Test thread", test_thread_entry, 0U, test_thread_pool, Config::TEST_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START); 
    tx_thread_create(&bq796xx_thread, (CHAR*)"BQ796XX thread", bq796xx_thread_entry, 0U, bq796xx_thread_pool, Config::BQ796xx_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&ads131m04_thread, (CHAR*)"ADS131M04 thread", ads131m04_thread_entry, 0U, ads131m04_thread_pool, Config::ADS131m04_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&main_thread, (CHAR*)"Main thread", main_thread_entry, 0U, main_thread_pool, Config::MAIN_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_com_thread, (CHAR*)"USB COM INIT thread", usb_com_thread_entry, 0U, usb_com_thread_pool, Config::USB_COM_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    // tx_thread_create(&usb_tx_thread, (CHAR*)"USB TX thread", usb_tx_thread_entry, 0U, usb_tx_thread_pool, usb_tx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    // tx_thread_create(&usb_rx_thread, (CHAR*)"USB RX thread", usb_rx_thread_entry, 0U, usb_rx_thread_pool, usb_rx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&car_can_thread, (CHAR*)"Other COM thread", car_can_thread_entry, 0U, car_can_thread_pool, Config::CAR_CAN_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&charger_can_thread, (CHAR*)"Charger COM thread", charger_thread_entry, 0U, charger_can_thread_pool, Config::CHARGER_CAN_THREAD_POOL_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_semaphore_create(&data_semaphore, (CHAR*)"Data semaphore", 0U);

    tx_timer_create(&soc_update_timer, (CHAR*)"SOC update timer", soc_update_timer_callback, 0U, 1000U, 1000U, TX_NO_ACTIVATE);
    
    //tx_thread_suspend(&charger_can_thread);
}
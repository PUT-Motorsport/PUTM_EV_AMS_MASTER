#include "threads.hpp"

#include "main.h"
#include "tx_api.h"

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
static constexpr size_t main_thread_pool_size { 1024U };
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

extern TX_MUTEX tx_buffer_mutex;
extern TX_MUTEX rx_buffer_mutex;

VOID init_static_threads()
{
    // tx_thread_create(test_thread, "Test thread", test_thread_entry, 0U, test_thread_pool, test_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&bq796xx_thread, (CHAR*)"BQ796XX thread", bq796xx_thread_entry, 0U, bq796xx_thread_pool, bq796xx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&ads131m04_thread, (CHAR*)"ADS131M04 thread", ads131m04_thread_entry, 0U, ads131m04_thread_pool, ads131m04_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&main_thread, (CHAR*)"Main thread", main_thread_entry, 0U, main_thread_pool, main_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_com_thread, (CHAR*)"USB -COM- INIT thread", usb_com_thread_entry, 0U, usb_com_thread_pool, usb_com_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_tx_thread, (CHAR*)"USB TX thread", usb_tx_thread_entry, 0U, usb_tx_thread_pool, usb_tx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&usb_rx_thread, (CHAR*)"USB RX thread", usb_rx_thread_entry, 0U, usb_rx_thread_pool, usb_rx_thread_pool_size, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);

    // tx_thread_suspend(&usb_com_thread);
    // tx_thread_suspend(&usb_tx_thread);
    // tx_thread_suspend(&usb_rx_thread);

    // /* init usb tx buffer mutex */
    // tx_mutex_create(&tx_buffer_mutex, "tx_buffer_mutex", 0);
    // /* init usb rx buffer mutex */
    // tx_mutex_create(&rx_buffer_mutex, "rx_buffer_mutex", 0);
}
#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"

#include "cstring"

extern "C"
{
#include "main.h"
#include "usb.h"
#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_cdc_acm.h"
}

#include "ArduinoJson.h"

using namespace PUTM::Data;
using namespace PUTM::Config;

static JsonDocument json;

static constexpr ULONG tx_rx_buffer_size { 512 };

static CHAR tx_buffer[tx_rx_buffer_size];
static CHAR rx_buffer[tx_rx_buffer_size];

static ULONG rx_actual_size { 0 };
static ULONG tx_actual_size { 0 };

extern TX_MUTEX tx_buffer_mutex;
extern TX_MUTEX rx_buffer_mutex;

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

VOID usb_com_thread_entry(__unused ULONG thread_input)
{
    /* give it some time? */
}

VOID usb_rx_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        if(cdc_acm != UX_NULL)
        {
            ux_device_class_cdc_acm_read(cdc_acm, (UCHAR *)rx_buffer, tx_rx_buffer_size, &rx_actual_size);
        }
        tx_thread_sleep(100);
    }
}

ULONG usbx_tx_status = 0;

VOID usb_tx_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        if(cdc_acm != UX_NULL)
        {
            json["current"] = PUTM::Data::current;
            json["acu_voltage"] = PUTM::Data::acu_voltage;
            json["car_voltage"] = PUTM::Data::car_voltage;
            json["soc"] = PUTM::Data::soc;

            serializeJson(json, tx_buffer, tx_rx_buffer_size);
            // TODO: func bellow block so the first transfer can be trash
            ux_device_class_cdc_acm_write(cdc_acm, (UCHAR *)(tx_buffer), strlen(tx_buffer), &tx_actual_size);
        }
        tx_thread_sleep(PUTM::Config::USB_TX_DELAY);
    }
}
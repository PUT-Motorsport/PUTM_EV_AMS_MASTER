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

using namespace PUTM;
using namespace PUTM::Config;

static StaticJsonDocument<512> json;

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
    tx_thread_sleep(200);
    MX_USB_PCD_Init();
    HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x00 , PCD_SNG_BUF, 0x40);
    HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x80 , PCD_SNG_BUF, 0x80);
    HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x01, PCD_SNG_BUF, 0xC0);
    HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x81, PCD_SNG_BUF, 0x100);
    HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x82, PCD_SNG_BUF, 0x140);
    ux_dcd_stm32_initialize((0), (ULONG)&hpcd_USB_DRD_FS);
    HAL_PCD_Start(&hpcd_USB_DRD_FS);

    // while(true)
    // {
    //     tx_thread_sleep(1000);
    // }
}

/**
 *  @brief  Usb rx thread
 *  @note   The `ux_device_class_cdc_acm_read()` function works in a blocking mode until a transfer request froma host computer is
 *          received 
 */
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
/**
 *  @brief  Usb tx thread
 *  @note   The `ux_device_class_cdc_acm_write()` function works in a blocking mode until a transfer request froma host computer is
 *          received 
 */
VOID usb_tx_thread_entry(__unused ULONG thread_input)
{
    while(true)
    {
        if(cdc_acm != UX_NULL)
        {
            json["timestamp"] = tx_time_get();
            json["current"] = data.current;
            json["acu_voltage"] = data.acu_voltage;
            json["car_voltage"] = data.car_voltage;
            json["soc"] = data.soc;

            // serializeJson(json, tx_buffer, tx_rx_buffer_size);
            serializeJsonPretty(json, tx_buffer, tx_rx_buffer_size);
            ux_device_class_cdc_acm_write(cdc_acm, (UCHAR *)(tx_buffer), strlen(tx_buffer), &tx_actual_size);
        }
        tx_thread_sleep(200);
    }
}
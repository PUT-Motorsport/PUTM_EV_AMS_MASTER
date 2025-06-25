#pragma once

#include "main.h"
#include "usart.h"
#include "tx_api.h"


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

struct Uart
{
private:
    UART_HandleTypeDef *huart;
    TX_SEMAPHORE semaphore;
    pUART_CallbackTypeDef tx_callback = [](UART_HandleTypeDef* huart)
    {
        if(huart->UserData == nullptr) Error_Handler();
        TX_SEMAPHORE *semaphore = (TX_SEMAPHORE*)huart->UserData;

        huart->TxCpltCallback = HAL_UART_TxCpltCallback;
        huart->UserData = nullptr;

        tx_semaphore_put(semaphore);
    };
    pUART_CallbackTypeDef rx_callback = [](UART_HandleTypeDef* huart)
    {
        if(huart->UserData == nullptr) Error_Handler();
        TX_SEMAPHORE *semaphore = (TX_SEMAPHORE*)huart->UserData;
        huart->RxCpltCallback = HAL_UART_RxCpltCallback;
        huart->UserData = nullptr;
        tx_semaphore_put(semaphore);   
    };
public:
    /**
     * 	@brief 	Constructor for Uart wrapper
     * 	@param 	`huart` pointer to UART_HandleTypeDef
     */
    Uart(UART_HandleTypeDef *huart) : huart(huart) { }
public:
    /**
     * 	@brief 	This function inits the UART wrapper
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef init()
    {
        if(tx_semaphore_create(&semaphore, "UART semaphore", 0) != TX_SUCCESS) return HAL_ERROR;
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function sets the UART baudrate
     * 	@param 	`baudrate` baudrate in bps
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef set_baudrate(uint32_t baudrate)
    {
        if(HAL_UART_DeInit(huart) != HAL_OK) return HAL_ERROR;
        huart->Init.BaudRate = baudrate;
        if(HAL_UART_Init(huart) != HAL_OK) return HAL_ERROR;
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function sets the UART receiver timeout
     * 	@param 	`baudblocks` receiver timeout in baudblocks
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef set_rx_timeout(uint32_t baudblocks)
    {
        HAL_UART_ReceiverTimeout_Config(huart, baudblocks);
        if(HAL_UART_EnableReceiverTimeout(huart) != HAL_OK) return HAL_ERROR;
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function aborts the UART transmission
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef abort()
    {
        return HAL_UART_Abort(huart);
    }
public:
    /**
     * 	@brief 	This function transmits data over UART
     * 	@param 	`tx_data` pointer to data to be transmitted
     * 	@param 	`size` size of data to be transmitted
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef tx(uint8_t *tx_data, size_t size)
    {
        if(HAL_UART_Transmit(huart, tx_data, size, 100) != HAL_OK) return HAL_ERROR;
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function receives data over UART
     * 	@param 	`rx_data` pointer to data to be received
     * 	@param 	`size` size of data to be received
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef rx(uint8_t *rx_data, size_t size)
    {
        if(HAL_UART_Receive(huart, rx_data, size, 100) != HAL_OK) return HAL_ERROR;
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function transmits and receives data over UART
     * 	@param 	`tx_data` pointer to data to be transmitted
     * 	@param 	`rx_data` pointer to data to be received
     * 	@param 	`size` size of data to be transmitted and received
     * 	@retval	HAL_OK
     */
    // HAL_StatusTypeDef tx_rx(uint8_t *tx_data, uint8_t *rx_data, size_t size)
    // {
    //     if(HAL_UART_TransmitReceive(huart, tx_data, rx_data, size, 100) != HAL_OK) return HAL_ERROR;
    //     return HAL_OK;
    // }
public:
    /**
     * 	@brief 	This function transmits data over UART in DMA mode
     * 	@param 	`tx_data` pointer to data to be transmitted
     * 	@param 	`size` size of data to be transmitted
     *  @param 	`timeout` timeout in threadex system ticks, default is 100
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef async_tx_dma(uint8_t *tx_data, size_t size, size_t timeout = 100)
    {
        huart->TxCpltCallback = tx_callback;
        huart->UserData = (void*)&semaphore;
        if(HAL_UART_Transmit_DMA(huart, tx_data, size) != HAL_OK) return HAL_ERROR;
        if(tx_semaphore_get(&semaphore, timeout) != 0) 
        { 
            HAL_UART_Abort(huart); 
            return HAL_TIMEOUT;
        }
        return HAL_OK;
    }
public:
    /**
     * 	@brief 	This function receives data over UART in DMA mode
     * 	@param 	`rx_data` pointer to data to be received
     * 	@param 	`size` size of data to be received
     *  @param 	`timeout` timeout in threadex system ticks, default is 100
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef async_rx_dma(uint8_t *rx_data, size_t size, size_t timeout = 100)
    {
        huart->RxCpltCallback = rx_callback;
        huart->UserData = (void*)&semaphore;
        if(HAL_UART_Receive_DMA(huart, rx_data, size) != HAL_OK) return HAL_ERROR;
        if(tx_semaphore_get(&semaphore, timeout) != 0) 
        { 
            HAL_UART_Abort(huart); 
            return HAL_TIMEOUT;
        }
        return HAL_OK;
    }
public: 
    /**
     * 	@brief 	This function transmits and receives data over UART in DMA mode
     * 	@param 	`tx_data` pointer to data to be transmitted
     *  @param 	`tx_size` size of data to be transmitted
     * 	@param 	`rx_data` pointer to data to be received
     *  @param 	`rx_size` size of data to be received
     *  @param 	`timeout` timeout in threadex system ticks, default is 100
     * 	@retval	HAL_OK
     */
    HAL_StatusTypeDef async_tx_rx_dma(uint8_t *tx_data, size_t tx_size, uint8_t *rx_data, size_t rx_size, size_t timeout = 100)
    {
        // huart->TxCpltCallback = tx_callback;
        huart->RxCpltCallback = rx_callback;
        huart->UserData = (void*)&semaphore;
        if(HAL_UART_Transmit_DMA(huart, tx_data, tx_size) != HAL_OK) return HAL_ERROR;
        if(HAL_UART_Receive_DMA(huart, rx_data, rx_size) != HAL_OK) return HAL_ERROR;
        if(tx_semaphore_get(&semaphore, timeout) != 0) 
        { 
            HAL_UART_Abort(huart); 
            return HAL_TIMEOUT;
        }
        return HAL_OK;
    }
};
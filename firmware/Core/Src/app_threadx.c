/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "threads.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define test_thread_pool_size 1024U
#define bq796xx_thread_pool_size 1024U
#define ams131m04_thread_pool_size 1024U
#define main_thread_pool_size 1024U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/*
*   @brief define thread with name with pool size
*   @note threads are staticaly allocated
*/
#define def_thread(NAME, SIZE)            \
TX_THREAD NAME##_thread;                  \
UCHAR NAME##_thread_pool[SIZE]            \

/*
*   @brief create thread of name with pool size
*   @note threads are staticaly created
*/
#define create_thread(NAME, SIZE)         \
tx_thread_create(&NAME##_thread, #NAME " thread", NAME##_thread_entry, 0x00, NAME##_thread_pool, SIZE, 20, 20, TX_NO_TIME_SLICE, TX_AUTO_START)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
// TX_THREAD test_thread;

def_thread(test, test_thread_pool_size);
def_thread(bq796xx, test_thread_pool_size);
def_thread(ads131m04, test_thread_pool_size);
def_thread(main, main_thread_pool_size);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  create_thread(test, test_thread_pool_size);
  create_thread(bq796xx, bq796xx_thread_pool_size);
  create_thread(ads131m04, ams131m04_thread_pool_size);
  create_thread(main, main_thread_pool_size);

  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

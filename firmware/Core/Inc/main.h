/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DET_CHARGER_Pin GPIO_PIN_13
#define DET_CHARGER_GPIO_Port GPIOC
#define DET_TSMS_Pin GPIO_PIN_14
#define DET_TSMS_GPIO_Port GPIOC
#define ADC_NDRY_Pin GPIO_PIN_0
#define ADC_NDRY_GPIO_Port GPIOC
#define INT_SPI_RDY_Pin GPIO_PIN_1
#define INT_SPI_RDY_GPIO_Port GPIOC
#define INT_SPI_RDY_EXTI_IRQn EXTI1_IRQn
#define NFLT_Pin GPIO_PIN_2
#define NFLT_GPIO_Port GPIOC
#define SIG_AMS_ERROR_Pin GPIO_PIN_3
#define SIG_AMS_ERROR_GPIO_Port GPIOC
#define FAN1_CTRL_Pin GPIO_PIN_0
#define FAN1_CTRL_GPIO_Port GPIOA
#define FAN2_CTRL_Pin GPIO_PIN_1
#define FAN2_CTRL_GPIO_Port GPIOA
#define FAN3_CTRL_Pin GPIO_PIN_2
#define FAN3_CTRL_GPIO_Port GPIOA
#define FAN4_CTRL_Pin GPIO_PIN_3
#define FAN4_CTRL_GPIO_Port GPIOA
#define ADC_NSS_Pin GPIO_PIN_4
#define ADC_NSS_GPIO_Port GPIOA
#define ADC_SCK_Pin GPIO_PIN_5
#define ADC_SCK_GPIO_Port GPIOA
#define ADC_MISO_Pin GPIO_PIN_6
#define ADC_MISO_GPIO_Port GPIOA
#define ADC_MOSI_Pin GPIO_PIN_7
#define ADC_MOSI_GPIO_Port GPIOA
#define SIG_AIR_PRE_Pin GPIO_PIN_5
#define SIG_AIR_PRE_GPIO_Port GPIOC
#define SIG_AIR_P_Pin GPIO_PIN_0
#define SIG_AIR_P_GPIO_Port GPIOB
#define SIG_AIR_M_Pin GPIO_PIN_1
#define SIG_AIR_M_GPIO_Port GPIOB
#define PG_12V_Pin GPIO_PIN_12
#define PG_12V_GPIO_Port GPIOB
#define EN_12V_Pin GPIO_PIN_13
#define EN_12V_GPIO_Port GPIOB
#define DET_AIR_M_Pin GPIO_PIN_14
#define DET_AIR_M_GPIO_Port GPIOB
#define DET_AIR_PRE_Pin GPIO_PIN_15
#define DET_AIR_PRE_GPIO_Port GPIOB
#define FAN5_CTRL_Pin GPIO_PIN_6
#define FAN5_CTRL_GPIO_Port GPIOC
#define FAN6_CTRL_Pin GPIO_PIN_7
#define FAN6_CTRL_GPIO_Port GPIOC
#define FAN7_CTRL_Pin GPIO_PIN_8
#define FAN7_CTRL_GPIO_Port GPIOC
#define DET_AIR_P_Pin GPIO_PIN_9
#define DET_AIR_P_GPIO_Port GPIOC
#define COM_NSS_Pin GPIO_PIN_15
#define COM_NSS_GPIO_Port GPIOA
#define COM_SCK_Pin GPIO_PIN_10
#define COM_SCK_GPIO_Port GPIOC
#define COM_MISO_Pin GPIO_PIN_11
#define COM_MISO_GPIO_Port GPIOC
#define COM_MOSI_Pin GPIO_PIN_12
#define COM_MOSI_GPIO_Port GPIOC
#define LED_ERROR_Pin GPIO_PIN_2
#define LED_ERROR_GPIO_Port GPIOD
#define LED_WARNING_Pin GPIO_PIN_3
#define LED_WARNING_GPIO_Port GPIOB
#define LED_OK_Pin GPIO_PIN_4
#define LED_OK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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
#define BLUE_PUSH_BUTTON_Pin GPIO_PIN_13
#define BLUE_PUSH_BUTTON_GPIO_Port GPIOC
#define BLUE_PUSH_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define PH0_MCU_Pin GPIO_PIN_0
#define PH0_MCU_GPIO_Port GPIOH
#define PH1_MCU_Pin GPIO_PIN_1
#define PH1_MCU_GPIO_Port GPIOH
#define FIBER_OPTIC_SENSOR_Pin GPIO_PIN_3
#define FIBER_OPTIC_SENSOR_GPIO_Port GPIOC
#define MOTOR_POWER_PWM_Pin GPIO_PIN_0
#define MOTOR_POWER_PWM_GPIO_Port GPIOA
#define MOTOR_DIRECTION_LEFT_Pin GPIO_PIN_1
#define MOTOR_DIRECTION_LEFT_GPIO_Port GPIOA
#define MOTOR_DIRECTION_RIGHT_Pin GPIO_PIN_2
#define MOTOR_DIRECTION_RIGHT_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_GPIO_Port GPIOB
#define ELECTROMAGNET_RIGHT_Pin GPIO_PIN_9
#define ELECTROMAGNET_RIGHT_GPIO_Port GPIOE
#define OPTICAL_ENDSTOP_RIGHT_Pin GPIO_PIN_14
#define OPTICAL_ENDSTOP_RIGHT_GPIO_Port GPIOE
#define OPTICAL_ENDSTOP_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define OPTICAL_ENDSTOP_LEFT_Pin GPIO_PIN_15
#define OPTICAL_ENDSTOP_LEFT_GPIO_Port GPIOE
#define OPTICAL_ENDSTOP_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define LED_RED_Pin GPIO_PIN_14
#define LED_RED_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define ELECTROMAGNET_LEFT_Pin GPIO_PIN_6
#define ELECTROMAGNET_LEFT_GPIO_Port GPIOC
#define POSITIONER_1_Pin GPIO_PIN_0
#define POSITIONER_1_GPIO_Port GPIOD
#define POSITIONER_1_EXTI_IRQn EXTI0_IRQn
#define POSITIONER_2_Pin GPIO_PIN_1
#define POSITIONER_2_GPIO_Port GPIOD
#define POSITIONER_2_EXTI_IRQn EXTI1_IRQn
#define POSITIONER_3_Pin GPIO_PIN_2
#define POSITIONER_3_GPIO_Port GPIOD
#define POSITIONER_3_EXTI_IRQn EXTI2_IRQn
#define POSITIONER_4_Pin GPIO_PIN_3
#define POSITIONER_4_GPIO_Port GPIOD
#define POSITIONER_4_EXTI_IRQn EXTI3_IRQn
#define POSITIONER_5_Pin GPIO_PIN_4
#define POSITIONER_5_GPIO_Port GPIOD
#define POSITIONER_5_EXTI_IRQn EXTI4_IRQn
#define POSITIONER_6_Pin GPIO_PIN_5
#define POSITIONER_6_GPIO_Port GPIOD
#define POSITIONER_6_EXTI_IRQn EXTI9_5_IRQn
#define POSITIONER_7_Pin GPIO_PIN_6
#define POSITIONER_7_GPIO_Port GPIOD
#define POSITIONER_7_EXTI_IRQn EXTI9_5_IRQn
#define POSITIONER_8_Pin GPIO_PIN_7
#define POSITIONER_8_GPIO_Port GPIOD
#define POSITIONER_8_EXTI_IRQn EXTI9_5_IRQn
#define LED_YELLOW_Pin GPIO_PIN_1
#define LED_YELLOW_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

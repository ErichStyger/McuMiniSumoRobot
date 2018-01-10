/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Prox_M_Pin GPIO_PIN_0
#define Prox_M_GPIO_Port GPIOF
#define Prox_L_Pin GPIO_PIN_1
#define Prox_L_GPIO_Port GPIOF
#define PWM_L_Pin GPIO_PIN_0
#define PWM_L_GPIO_Port GPIOA
#define PWM_R_Pin GPIO_PIN_1
#define PWM_R_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define Dir_A_Pin GPIO_PIN_3
#define Dir_A_GPIO_Port GPIOA
#define Dir_B_Pin GPIO_PIN_4
#define Dir_B_GPIO_Port GPIOA
#define Prox_Ir_Select_Pin GPIO_PIN_5
#define Prox_Ir_Select_GPIO_Port GPIOA
#define Enc_R_A_Pin GPIO_PIN_6
#define Enc_R_A_GPIO_Port GPIOA
#define Enc_R_B_Pin GPIO_PIN_7
#define Enc_R_B_GPIO_Port GPIOA
#define Enc_L_A_Pin GPIO_PIN_0
#define Enc_L_A_GPIO_Port GPIOB
#define Enc_L_B_Pin GPIO_PIN_1
#define Enc_L_B_GPIO_Port GPIOB
#define Edge_R_Pin GPIO_PIN_8
#define Edge_R_GPIO_Port GPIOA
#define Edge_MR_Pin GPIO_PIN_11
#define Edge_MR_GPIO_Port GPIOA
#define Prox_R_Pin GPIO_PIN_12
#define Prox_R_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define Usr_But_Pin GPIO_PIN_3
#define Usr_But_GPIO_Port GPIOB
#define Edge_L_Pin GPIO_PIN_4
#define Edge_L_GPIO_Port GPIOB
#define Edge_ML_Pin GPIO_PIN_5
#define Edge_ML_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
 * PWM.c
 *
 *  Created on: 13.12.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "PWM.h"
#include "stm32f3xx_hal.h"

#define  PERIOD_VALUE       	(uint32_t)(640-1)  /* Period Value: timer at 64 MHz, 640 ticks are 10us ==> 100 kHz PWM  */
#define  PULSE1_INIT_VALUE      (uint32_t)(0)      /* Initial duty 0%, Capture Compare 1 Value  */
#define  PULSE2_INIT_VALUE      (uint32_t)(0)      /* Initial duty 0%, Capture Compare 2 Value  */

static TIM_HandleTypeDef htim2; /* timer handle for TIM2 */

void PWM_SetValue16(uint16_t value, uint32_t channel) {
    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    if (value==0) {
    	sConfigOC.Pulse = 0;
    } else if (value==0xffff) {
    	sConfigOC.Pulse = 0xffff;
    } else {
        sConfigOC.Pulse = ((value*PERIOD_VALUE)/0xffff);
    }
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, channel);
    HAL_TIM_PWM_Start(&htim2, channel);
}

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
  #define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOA_CLK_ENABLE(); \
                                   __HAL_RCC_GPIOB_CLK_ENABLE();
  #define TIMx_GPIO_PORT_CHANNEL1        GPIOA
  #define TIMx_GPIO_PORT_CHANNEL2        GPIOA
  #define TIMx_GPIO_AF_CHANNEL1          GPIO_AF1_TIM2
  #define TIMx_GPIO_AF_CHANNEL2          GPIO_AF1_TIM2
  #define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_0
  #define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_1
  #define TIMx_CLK_ENABLE()              __HAL_RCC_TIM2_CLK_ENABLE()

  GPIO_InitTypeDef   GPIO_InitStruct;
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* TIMx Peripheral clock enable */
  TIMx_CLK_ENABLE();

  /* Enable all GPIO Channels Clock requested */
  TIMx_CHANNEL_GPIO_PORT();

  /* Configure PA.0 (TIM2_Channel1), PA.01 (TIM2_Channel2)
   * in output, push-pull, alternate function mode
  */
  /* Common configuration for all channels */
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL1;
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

  GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL2;
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);
}

/* TIM2 init function */
static void MX_TIM2_Init(void) {
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  //uint32_t uhPrescalerValue = 0;
  /* Timer Output Compare Configuration Structure declaration */
  TIM_OC_InitTypeDef sConfig;

  /*##-1- Configure the TIM peripheral #######################################*/
  /* -----------------------------------------------------------------------
  TIM2 Configuration: generate 4 PWM signals with 4 different duty cycles.

  In this example TIM2 input clock (TIM2CLK) is set to 2 * APB1 clock (PCLK1),
    since APB1 prescaler is different from 1.
      TIM2CLK = 2 * PCLK1
      PCLK1 = HCLK / 2
      => TIM2CLK = HCLK = SystemCoreClock

    To get TIM2 counter clock at 16 MHz, the prescaler is computed as follows:
       Prescaler = (TIM3CLK / TIM2 counter clock) - 1
       Prescaler = (SystemCoreClock /16 MHz) - 1

    To get TIM3 output clock at 36 KHz, the period (ARR)) is computed as follows:
       ARR = (TIM3 counter clock / TIM3 output clock) - 1
           = 443

    TIM2 Channel1 duty cycle = (TIM2_CCR1/ TIM2_ARR)* 100 = 50%
    TIM2 Channel2 duty cycle = (TIM2_CCR2/ TIM2_ARR)* 100 = 37.5%

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f3xx.c file.
     Each time the core clock (HCLK) changes, user had to update SystemCoreClock
     variable value. Otherwise, any configuration based on this variable will be incorrect.
     This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  ----------------------------------------------------------------------- */

  /* Initialize TIMx peripheral as follows:
       + Prescaler = (SystemCoreClock / 16000000) - 1
       + Period = (444 - 1)
       + ClockDivision = 0
       + Counter direction = Up
  */

  /* Compute the prescaler value to have TIM2 counter clock equal to 16000000 Hz */
  //uhPrescalerValue = (uint32_t)(SystemCoreClock / 16000000) - 1;
  /* SystemCoreClock is 64 MHz */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0; /* running it with 64 MHz */ //uhPrescalerValue;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = PERIOD_VALUE;
  htim2.Init.ClockDivision = 0;
  htim2.Init.RepetitionCounter = 0;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
	  Error_Handler();
  }
  /*##-2- Configure the PWM channels #########################################*/
  /* Common configuration for all channels */
  sConfig.OCMode       = TIM_OCMODE_PWM1;
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

  /* Set the pulse value for channel 1 */
  sConfig.Pulse = PULSE1_INIT_VALUE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Configuration Error */
    Error_Handler();
  }

  /* Set the pulse value for channel 2 */
  sConfig.Pulse = PULSE2_INIT_VALUE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_2) != HAL_OK)
  {
    /* Configuration Error */
    Error_Handler();
  }
  /*##-3- Start PWM signals generation #######################################*/
  /* Start channel 1 */
  if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
    /* PWM Generation Error */
    Error_Handler();
  }
  /* Start channel 2 */
  if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    /* PWM Generation Error */
    Error_Handler();
  }
}

void PWM_Init(void) {
	MX_TIM2_Init();
}

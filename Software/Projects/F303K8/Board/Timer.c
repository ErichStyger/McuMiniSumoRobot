/**
 * \file
 * \brief Implementation of timer.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements to functions to use the timer module.
 */
#include "Platform.h"
#include "Timer.h"
#include "stm32f3xx_hal.h"
#include "Quadrature.h"
#include "Pin.h"

#if PL_CONFIG_HAS_QUADRATURE
TIM_HandleTypeDef htim1; /* quadrature sampling timer */
#endif
#if PL_CONFIG_HAS_REFLECTANCE
TIM_HandleTypeDef htim3; /* reflectance sensor measurement timer */
#endif

void TMR_OnInterrupt(TIM_HandleTypeDef *htim) {
#if PL_CONFIG_HAS_QUADRATURE
  if (htim==&htim1) {
	  QUAD_Sample();
	  //PIN_Toggle(PIN_DIR_L);
  }
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  if (htim==&htim3) {
	  //PIN_Toggle(PIN_DIR_R);
  }
#endif
}

#if PL_CONFIG_HAS_QUADRATURE
/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0; /* APB1 peripheral clock should be 64 MHz. */
  htim1.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim1.Init.Period = (6400-1); /* 6400 ticks @ 64 MHz ==> 100us  */
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}
#endif

#if PL_CONFIG_HAS_REFLECTANCE
/* TIM3 init function */
static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = (10*6400-1); /* 10*6400 ticks @ 64 MHz ==> 10*100us  */
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}
#endif

/* Reflectance timer ***************************************/
#if PL_CONFIG_HAS_REFLECTANCE
void TMRR_Start(void) {
  HAL_TIM_Base_Start(&htim3); /* start timer */
}

void TMRR_Stop(void) {
  HAL_TIM_Base_Stop(&htim3); /* stop timer interrupts */
}

uint32_t TMRR_SetCounter(uint32_t value) {
  return __HAL_TIM_SET_COUNTER(&htim3, value); /* return timer counter */
}

uint32_t TMRR_GetCounter(void) {
  return __HAL_TIM_GET_COUNTER(&htim3); /* return timer counter */
}
#endif
/* Quadrature timer ***************************************/
#if PL_CONFIG_HAS_QUADRATURE
void TMRQ_Start(void) {
  HAL_TIM_Base_Start(&htim1); /* start timer */
}

void TMRQ_Stop(void) {
  HAL_TIM_Base_Stop(&htim1); /* stop timer interrupts */
}

void TMRQ_StartInterrupts(void) {
  HAL_TIM_Base_Start_IT(&htim1); /* start timer interrupts */
}

void TMRQ_StopInterrupts(void) {
  HAL_TIM_Base_Stop_IT(&htim1);   /* stop timer */
}

uint32_t TMRQ_GetCounter(void) {
  return __HAL_TIM_GET_COUNTER(&htim1); /* return timer counter */
}
#endif

void TMR_Init(void) {
#if PL_CONFIG_HAS_QUADRATURE
  MX_TIM1_Init();
  TMRQ_StartInterrupts();
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  MX_TIM3_Init();
  //TMRR_Start();
#endif
}

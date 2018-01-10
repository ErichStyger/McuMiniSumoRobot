/**
 * \file
 * \brief Interface to the timer on the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements to functions to use the timer module.
 */

#ifndef BOARD_TIMER_H_
#define BOARD_TIMER_H_

#include <stdint.h>
#include "stm32f3xx_hal.h" /* for TIM_HandleTypeDef */

/** @addtogroup Timer
  * @{
  */

/*!
 * \brief Called for timer interrupts
 * \param htim Timer handle
 */
void TMR_OnInterrupt(TIM_HandleTypeDef *htim);

/*!
 * \brief Start the quadrature timer interrupts
 */
void TMRQ_StartInterrupts(void);

/*!
 * \brief Stop the quadrature timer interrupts
 */
void TMRQ_StopInterrupts(void);

/*!
 * \brief Get the quadrature timer counter register
 * \return Timer counter register value
 */
uint32_t TMRQ_GetCounter(void);

/*!
 * \brief Start quadrature timer with no interrupts
 */
void TMRQ_Start(void);

/*!
 * \brief Stop quadrature timer with no interrupts
 */
void TMRQ_Stop(void);

/* reflectance sensor timer */
void TMRR_Start(void);
void TMRR_Stop(void);
uint32_t TMRR_SetCounter(uint32_t value);
uint32_t TMRR_GetCounter(void);

/*!
 * \brief Timer initialization routine
 */
void TMR_Init(void);

/**
  * @}
  */

#endif /* BOARD_TIMER_H_ */

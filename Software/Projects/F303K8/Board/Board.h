/**
 * \file
 * \brief Interface to the board support package of the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the board support package.
 */

#ifndef SRC_BOARD_H_
#define SRC_BOARD_H_

/** @addtogroup Board
  * @{
  */

#include "Platform.h"
#if PL_CONFIG_HAS_HW_I2C
  #include "stm32f3xx_hal_i2c.h"
  extern I2C_HandleTypeDef hi2c1;
#endif

/*!
 * \brief Board main initialization routine
 */
void BOARD_Init(void);

/**
  * @}
  */

#endif /* SRC_BOARD_H_ */

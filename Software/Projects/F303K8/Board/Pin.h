/**
 * \file
 * \brief Interface to the pins on the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 * This module implements the functions to use the GPIO pins.
 */

#ifndef BOARD_PIN_H_
#define BOARD_PIN_H_

#include <stdbool.h>

/** @addtogroup Pin
  * @{
  */
#include "stm32f3xx_hal.h"

#define PIN_SW3_PORT  		GPIOB
#define PIN_SW3_PIN   		GPIO_PIN_3

#define PIN_DIRL_PORT  		GPIOA
#define PIN_DIRL_PIN   		GPIO_PIN_3

#define PIN_DIRR_PORT  		GPIOA
#define PIN_DIRR_PIN   		GPIO_PIN_4

#define PIN_ENCL_A_PORT  	GPIOB
#define PIN_ENCL_A_PIN   	GPIO_PIN_0

#define PIN_ENCL_B_PORT  	GPIOB
#define PIN_ENCL_B_PIN   	GPIO_PIN_1

#define PIN_ENCR_A_PORT  	GPIOA
#define PIN_ENCR_A_PIN   	GPIO_PIN_6

#define PIN_ENCR_B_PORT  	GPIOA
#define PIN_ENCR_B_PIN   	GPIO_PIN_7

#define PIN_PROX_IR_SELECT_PORT  	GPIOA
#define PIN_PROX_IR_SELECT_PIN   	GPIO_PIN_5

#define PIN_PROX_L_PORT  	GPIOF
#define PIN_PROX_L_PIN   	GPIO_PIN_1

#define PIN_PROX_M_PORT  	GPIOF
#define PIN_PROX_M_PIN   	GPIO_PIN_0

#define PIN_PROX_R_PORT  	GPIOA
#define PIN_PROX_R_PIN   	GPIO_PIN_12

#define PIN_PROX_R_PORT  	GPIOA
#define PIN_PROX_R_PIN   	GPIO_PIN_12

#define PIN_EDGE_L_PORT  	GPIOB
#define PIN_EDGE_L_PIN   	GPIO_PIN_4

#define PIN_EDGE_ML_PORT  	GPIOB
#define PIN_EDGE_ML_PIN   	GPIO_PIN_5

#define PIN_EDGE_MR_PORT  	GPIOA
#define PIN_EDGE_MR_PIN   	GPIO_PIN_11

#define PIN_EDGE_R_PORT  	GPIOA
#define PIN_EDGE_R_PIN   	GPIO_PIN_8

#if PL_CONFIG_HAS_I2C
#define PIN_I2C_SDA_PORT  	GPIOB
#define PIN_I2C_SDA_PIN   	GPIO_PIN_7

#define PIN_I2C_SCL_PORT  	GPIOB
#define PIN_I2C_SCL_PIN   	GPIO_PIN_6
#endif

/*!
 * \brief Identifiers for the Pins on the board
 */
typedef enum {
  PIN_SW3, /*!< User Switch SW3 */
  PIN_ENCL_A,
  PIN_ENCL_B,
  PIN_ENCR_A,
  PIN_ENCR_B,
  PIN_PROX_IR_SELECT,
  PIN_PROX_L,
  PIN_PROX_M,
  PIN_PROX_R,
  PIN_EDGE_L,
  PIN_EDGE_ML,
  PIN_EDGE_MR,
  PIN_EDGE_R,
  PIN_DIR_L,
  PIN_DIR_R,
#if PL_CONFIG_HAS_I2C
  PIN_I2C_SDA,
  PIN_I2C_SCL
#endif
} Pin_PinId;

/*!
 * \brief Function used to check if a pin is high or low.
 * \param pin Pin to be used
 * \return true if the pin has a high level, false otherwise
 */
bool PIN_IsPinHigh(Pin_PinId pin);

bool PIN_IsPinLow(Pin_PinId pin);

void PIN_SetHigh(Pin_PinId pin);
void PIN_SetLow(Pin_PinId pin);
void PIN_Toggle(Pin_PinId pin);
void PIN_SetDirection(Pin_PinId pin, bool isOutput);

/*!
 * \brief Pin driver initialization routine
 */
void PIN_Init(void);

/**
  * @}
  */

#endif /* BOARD_PIN_H_ */

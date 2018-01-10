/*
 * DIRR.c
 *
 *  Created on: 30.11.2017
 *      Author: Erich Styger
 */

#include "McuLib.h"
#include "DIRR.h"
#include "stm32f3xx_hal.h"
#include "Pin.h"

uint8_t DIRR_PutVal(bool val) {
	HAL_GPIO_WritePin(PIN_DIRR_PORT, PIN_DIRR_PIN,  val?GPIO_PIN_SET:GPIO_PIN_RESET);
	return ERR_OK; /*! \todo */
}

/*
 * DIRL.c
 *
 *  Created on: 30.11.2017
 *      Author: Erich Styger
 */

#include "McuLib.h"
#include "DIRL.h"
#include "Pin.h"
#include "stm32f3xx_hal.h"

uint8_t DIRL_PutVal(bool val) {
	HAL_GPIO_WritePin(PIN_DIRL_PORT, PIN_DIRL_PIN,  val?GPIO_PIN_SET:GPIO_PIN_RESET);
	return ERR_OK; /*! \todo */
}

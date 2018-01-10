/*
 * PWML.c
 *
 *  Created on: 30.11.2017
 *      Author: Erich Styger
 */

#include "McuLib.h"
#include "PWML.h"
#include "PWM.h"

uint8_t PWML_SetRatio16(uint16_t val) {
	PWM_SetValue16(val, PWM_CHANNEL_1);
	return ERR_OK; /*! \todo */
}

uint8_t PWML_Enable(void) {
	return ERR_OK; /*! \todo */
}

/*
 * SDA1.c
 *
 *  Created on: 23.12.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#if PL_CONFIG_HAS_SW_I2C
#include "SDA1.h"
#include "Pin.h"


void SDA1_SetDir(bool isOutput) {
	PIN_SetDirection(PIN_I2C_SDA, isOutput);
}

void SDA1_ClrVal(void){
	PIN_SetLow(PIN_I2C_SDA);
}

void SDA1_SetVal(void) {
	PIN_SetHigh(PIN_I2C_SDA);
}

bool SDA1_GetVal(void) {
	return PIN_IsPinHigh(PIN_I2C_SDA);;
}

void SDA1_Init(void) {
	PIN_SetDirection(PIN_I2C_SDA, false);
}

void SDA1_Deinit(void) {}

#endif /* #if PL_CONFIG_HAS_SW_I2C */

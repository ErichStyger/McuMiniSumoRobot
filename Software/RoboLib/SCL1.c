/*
 * SCL1.c
 *
 *  Created on: 23.12.2017
 *      Author: Erich Styger
 */
#include "Platform.h"
#if PL_CONFIG_HAS_SW_I2C
#include "SCL1.h"
#include "Pin.h"

void SCL1_SetDir(bool isOutput) {
	PIN_SetDirection(PIN_I2C_SCL, isOutput);
}

void SCL1_ClrVal(void){
	PIN_SetLow(PIN_I2C_SCL);
}

void SCL1_SetVal(void) {
	PIN_SetHigh(PIN_I2C_SCL);
}

bool SCL1_GetVal(void) {
	return PIN_IsPinHigh(PIN_I2C_SCL);
}

void SCL1_Init(void) {
	PIN_SetDirection(PIN_I2C_SCL, false);
}

void SCL1_Deinit(void) {}

#endif /* PL_CONFIG_HAS_SW_I2C */

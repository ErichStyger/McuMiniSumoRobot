/*
 * SDA1.h
 *
 *  Created on: 23.12.2017
 *      Author: Erich Styger
 */

#ifndef SRC_SDA1_H_
#define SRC_SDA1_H_

#include <stdbool.h>

void SDA1_ClrVal();
void SDA1_SetVal();
bool SDA1_GetVal();
void SDA1_SetDir(bool isOutput);

void SDA1_Init(void);
void SDA1_Deinit(void);


#endif /* SRC_SDA1_H_ */

/*
 * SCL1.h
 *
 *  Created on: 23.12.2017
 *      Author: Erich Styger
 */

#ifndef SRC_SCL1_H_
#define SRC_SCL1_H_

#include <stdbool.h>

void SCL1_SetDir(bool isOutput);
void SCL1_ClrVal(void);
void SCL1_SetVal(void);
bool SCL1_GetVal(void);

void SCL1_Init(void);
void SCL1_Deinit(void);

#endif /* SRC_SCL1_H_ */

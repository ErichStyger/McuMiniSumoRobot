/*
 * PWM.h
 *
 *  Created on: 13.12.2017
 *      Author: Erich Styger
 */

#ifndef SRC_PWM_H_
#define SRC_PWM_H_

#define PWM_CHANNEL_1 (0x0000U)
#define PWM_CHANNEL_2 (0x0004U)


void PWM_SetValue16(uint16_t value, uint32_t Channel);

void PWM_Init(void);

#endif /* SRC_PWM_H_ */

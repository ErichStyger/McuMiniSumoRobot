/*
 * Line.h
 *
 *  Created on: 09.01.2018
 *      Author: Erich Styger
 */

#ifndef SRC_ROBOT_LINE_H_
#define SRC_ROBOT_LINE_H_

#include "Platform.h"
#if PL_CONFIG_HAS_LINE
#include "Reflectance.h"
#include <stdint.h>
#include <stdbool.h>

#if PL_CONFIG_HAS_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t LINE_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

uint16_t LINE_GetLinePos(void);

void LINE_CalibrateStartStop(void);
bool LINE_IsCalibrating(void);

REF_SensorTimeType LINE_Get1kValue(unsigned int idx);

void LINE_StateMachine(void);

void LINE_Init(void);

#endif /* PL_CONFIG_HAS_LINE */

#endif /* SRC_ROBOT_LINE_H_ */

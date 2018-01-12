/*
 * Sumo.h
 *
 *  Created on: 19.12.2017
 *      Author: Erich Styger
 */

#ifndef SRC_ROBOT_SUMO_H_
#define SRC_ROBOT_SUMO_H_

#if PL_CONFIG_HAS_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t SUMO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

bool SUMO_IsDoingSumo(void);

void SUMO_StartSumo(void);

void SUMO_StopSumo(void);

void SUMO_StartStopSumo(void);

int16_t SUMO_GetCountDownMs(void);

void SUMO_Init(void);

#endif /* SRC_ROBOT_SUMO_H_ */

/**
 * \file
 * \brief Proximity Module
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module handles the proximity (IR) sensors to detect an obstacle or another robot.
 */

#ifndef SRC_PROXIMITY_H_
#define SRC_PROXIMITY_H_

#include "Platform.h"
#include <stdint.h>

#if PL_CONFIG_HAS_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t PROX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

bool PROX_HasTarget(void);

typedef enum {
	PROX_L_LEFT_BIT=(1<<5),    /*!< left IR on, and seen on left sensor */
	PROX_L_MIDDLE_BIT=(1<<4),  /*!< left IR on, and seen on middle sensor */
  PROX_L_RIGHT_BIT=(1<<3),   /*!< right IR on, and seen on right sensor? */
  PROX_R_LEFT_BIT=(1<<2),    /*!< right IR on, and seen on left sensor? */
	PROX_R_MIDDLE_BIT=(1<<1),  /*!< right IR on, and seen on middle sensor */
	PROX_R_RIGHT_BIT=(1<<0),   /*!< right IR on, and seen on right sensor */
} PROX_Bits;

PROX_Bits PROX_GetProxBits(void);

#define PROX_NOF_SENSORS   (3)

/* return number of brightness levels for each sensor */
uint8_t PROX_GetNofWithLeftLeds(uint8_t sensorIdx);
uint8_t PROX_GetNofWithRightLeds(uint8_t sensorIdx);

/*!
 * \brief checks for the opponent
 * \return angle where the opponent is seen in degrees (e.g. -180 means left, 90 on the right side, 0 in front,
 * -1 if no target seen
 */
int PROX_GetTargetAngle(void);

void PROX_Init(void);

#endif /* SRC_PROXIMITY_H_ */

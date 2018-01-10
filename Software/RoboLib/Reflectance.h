/*
 * Reflectance.h
 *
 *  Created on: 15.12.2017
 *      Author: Erich Styger
 */

#ifndef SRC_REFLECTANCE_H_
#define SRC_REFLECTANCE_H_

#include "Platform.h"
#include <stdint.h>

#define REF_NOF_SENSORS       (4)
typedef uint16_t REF_SensorTimeType;
#define REF_MAX_SENSOR_VALUE  ((REF_SensorTimeType)-1)

#if PL_CONFIG_HAS_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t REF_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Returns a bit set of sensor values if 'white' or not
 * \return Bit set for 'white' seen
 */
uint32_t REF_IsWhite(void);

REF_SensorTimeType REF_GetRawValue(unsigned int idx);

void REF_Init(void);

#endif /* SRC_REFLECTANCE_H_ */

/**
 * \file
 * \brief Quadrature encoder
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements decoding the quadrature position encoders.
 */

#ifndef SRC_QUADRATURE_H_
#define SRC_QUADRATURE_H_

#include <stdint.h>
#include "Platform.h"

#define QUAD_CNTR_BITS  (32)
typedef uint32_t QUAD_QuadCntrType;

void QUAD_Sample(void);

void QUAD_Reset(void);
void QUAD_SetLeftPos(QUAD_QuadCntrType pos);
void QUAD_SetRightPos(QUAD_QuadCntrType pos);

QUAD_QuadCntrType QUAD_GetLeftPos(void);
QUAD_QuadCntrType QUAD_GetRightPos(void);
uint32_t QUAD_NofLeftErrors(void);
uint32_t QUAD_NofRightErrors(void);

#if PL_CONFIG_HAS_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t QUAD_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

void QUAD_Init(void);

#endif /* SRC_QUADRATURE_H_ */

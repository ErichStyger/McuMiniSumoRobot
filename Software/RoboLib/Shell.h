/**
 * \file
 * \brief Shell command line interface.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements a command line interface to the application.
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "Platform.h"
#if PL_CONFIG_HAS_SHELL

#include "McuShell.h"

/*!
 * \brief Sends a string to be parsed to the shell
 * \param cmd String to be parsed
 */
void SHELL_ParseCmd(uint8_t *cmd);

/*!
 * \brief Sends a string to the shell/console stdout
 * \param msg Zero terminated string to write
 */
void SHELL_SendString(unsigned char *msg);

/*! \brief Shell Module initialization, creates Shell task */
void SHELL_Init(void);

/*! \brief Shell driver de-initialization */
void SHELL_Deinit(void);

#endif /* PL_CONFIG_HAS_SHELL */

#endif /* SHELL_H_ */

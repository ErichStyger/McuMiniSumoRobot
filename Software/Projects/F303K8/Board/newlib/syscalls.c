/*
 * SysCalls.c
 *
 *  Created on: 24.11.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "McuShell.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>

/* forward declarations to avoid compiler warnings */
int _write(int iFileHandle __attribute__((unused)), char *pcBuffer, int iLength);
int _read(void);
int _isatty(int file __attribute__((unused)));
int __attribute__((weak)) _close(int fildes __attribute__((unused)));
int __attribute__((weak)) _fstat (int fd __attribute__((unused)), struct stat *s);
int __attribute__((weak)) _lseek(int file __attribute__((unused)), int ptr __attribute__((unused)), int dir __attribute__((unused)));

int _write(int iFileHandle __attribute__((unused)), char *pcBuffer, int iLength) {
  int i;

  for (i = 0; i < iLength; i++) {
	  McuShell_SendChar((uint8_t)pcBuffer[i]);
  }
  return iLength;
}

int _read(void) {
	uint8_t c;
	McuShell_ReadChar(&c);
	if (c=='\0') {
		return EOF;
	}
	return (int)c;
}

int _isatty(int file __attribute__((unused))) {
  return 1;
}


int __attribute__((weak)) _close(int fildes __attribute__((unused))) {
  return -1;
}

int __attribute__((weak)) _lseek(int file __attribute__((unused)), int ptr __attribute__((unused)), int dir __attribute__((unused))) {
  return -1;
}

int __attribute__((weak)) _fstat (int fd __attribute__((unused)), struct stat *st)
{
  memset (st, 0, sizeof(*st));
  st->st_mode = S_IFCHR;
  return 0;
}


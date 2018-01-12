/**
 * \file
 * \brief Main entry point of the application.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements main() and configures the hardware.
 */

#include "Platform.h"
#include "main.h"
#include "Application.h"

#include "string.h"

/*!
 * \brief main application entry point
 * \return error code, 0 for no error
 */
int main(void) {
  /* PL_Init(); */ /* already called during startup! initialization of platform, board and drivers */
  APP_Run();
  for(;;) {
	  /* should not get here! */
  }
  return 0; /* main() never should return! */
}

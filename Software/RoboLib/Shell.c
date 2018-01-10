/**
 * \file
 * \brief Shell and console interface implementation.
 * \author Erich Styger
 *
 * This module implements the front to the console/shell functionality.
 */

#include "Platform.h"
#if PL_CONFIG_HAS_SHELL
#include "Shell.h"
#include "McuShell.h"
#include "McuUtility.h"
#include "Application.h"
#if PL_CONFIG_USE_FREERTOS
  #include "FreeRTOS.h"
  #include "McuRTOS.h"
#endif
#if PL_CONFIG_HAS_MOTOR
  #include "Motor.h"
#endif
#if PL_CONFIG_HAS_QUADRATURE
  #include "Quadrature.h"
#endif
#if PL_CONFIG_HAS_PROXIMITY
  #include "Proximity.h"
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  #include "Reflectance.h"
#endif
#if PL_CONFIG_HAS_PID
  #include "Pid.h"
#endif
#if PL_CONFIG_HAS_MOTOR_TACHO
  #include "Tacho.h"
#endif
#if PL_CONFIG_HAS_DRIVE
  #include "Drive.h"
#endif
#if PL_CONFIG_HAS_SUMO
  #include "Sumo.h"
#endif
#if PL_CONFIG_HAS_TURN
  #include "Turn.h"
#endif
#if PL_CONFIG_HAS_LINE
  #include "Line.h"
#endif
#include "McuArmTools.h"

static uint8_t SHELL_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */

typedef struct {
  McuShell_ConstStdIOType *stdio;
  unsigned char *buf;
  size_t bufSize;
} SHELL_IODesc;

static const SHELL_IODesc ios[] =
{
  {&McuShell_stdio, SHELL_DefaultShellBuffer, sizeof(SHELL_DefaultShellBuffer)},
};

static const McuShell_ParseCommandCallback CmdParserTable[] =
{
  McuShell_ParseCommand, /* Processor Expert Shell component, is first in list */
#if McuRTOS_PARSE_COMMAND_ENABLED
  McuRTOS_ParseCommand, /* FreeRTOS shell parser */
#endif
#if McuArmTools_CONFIG_PARSE_COMMAND_ENABLED
  McuArmTools_ParseCommand,
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  REF_ParseCommand,
#endif
#if PL_CONFIG_HAS_PROXIMITY
  PROX_ParseCommand,
#endif
#if PL_CONFIG_HAS_MOTOR
  MOT_ParseCommand,
#endif
#if PL_CONFIG_HAS_QUADRATURE
  QUAD_ParseCommand,
#endif
#if PL_CONFIG_HAS_MOTOR_TACHO
  TACHO_ParseCommand,
#endif
#if PL_CONFIG_HAS_PID
  PID_ParseCommand,
#endif
#if PL_CONFIG_HAS_DRIVE
  DRV_ParseCommand,
#endif
#if PL_CONFIG_HAS_TURN
  TURN_ParseCommand,
#endif
#if PL_CONFIG_HAS_LINE
  LINE_ParseCommand,
#endif
#if PL_CONFIG_HAS_LINE_FOLLOW
  LF_ParseCommand,
#endif
#if PL_CONFIG_HAS_LINE_MAZE
  MAZE_ParseCommand,
#endif
#if PL_CONFIG_HAS_SUMO
  SUMO_ParseCommand,
#endif
  NULL /* Sentinel */
};

#if PL_CONFIG_USE_FREERTOS
static void ShellTask(void *pvParameters) {
  int i;

  (void)pvParameters; /* not used */
  /* initialize buffers */
  for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0';
  }
  McuShell_SendStr((uint8_t*)"Shell task started!\r\n", ios[0].stdio->stdOut);
  (void)McuShell_ParseWithCommandTable((unsigned char*)McuShell_CMD_HELP, ios[0].stdio, CmdParserTable);
  for(;;) {
    /* process all I/Os */
    for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
      (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  } /* for */
}
#endif /* PL_CONFIG_HAS_RTOS */

void SHELL_Init(void) {
#if PL_CONFIG_USE_FREERTOS
  if (xTaskCreate(ShellTask, "Shell", 900/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#endif
}

void SHELL_Deinit(void) {
}

#endif /* PL_CONFIG_HAS_SHELL */

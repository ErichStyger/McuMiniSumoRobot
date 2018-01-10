/**
 * \file
 * \brief Implementation Platform initialization of the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the hardware dependent initialization of board and drivers.
 */

#include "Platform.h"
#include "Board.h"
#include "McuWait.h"
#include "McuRTT.h"
#include "McuUtility.h"
#if PL_CONFIG_USE_FREERTOS
  #include "McuRTOS.h"
  #include "Application.h"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#elif configUSE_PERCEPIO_TRACE_HOOKS
  #include "McuPercepio.h"
#endif
#if PL_CONFIG_HAS_SW_I2C
  #include "McuGenericSWI2C.h"
#endif
#if PL_CONFIG_HAS_MOTOR
  #include "Motor.h"
  #include "PWM.h"
#endif
#if PL_CONFIG_HAS_PROXIMITY
  #include "Proximity.h"
#endif
#if PL_CONFIG_HAS_QUADRATURE
  #include "Quadrature.h"
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  #include "Reflectance.h"
#endif
#if PL_CONFIG_HAS_TIMER
  #include "Timer.h"
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
#if PL_CONFIG_HAS_TURN
  #include "Turn.h"
#endif
#if PL_CONFIG_HAS_SHELL
  #include "McuShell.h"
  #include "Shell.h"
#endif
#if PL_CONFIG_HAS_LCD
  #include "McuSSD1306.h"
  #include "McuGDisplaySSD1306.h"
  #include "LCD.h"
#endif
#if PL_CONFIG_HAS_I2C
  #include "McuGenericI2C.h"
#endif
#if PL_CONFIG_HAS_SW_I2C
  #include "McuGenericSWI2C.h"
#endif
#if PL_CONFIG_HAS_HW_I2C
  #include "stm32f3xx_hal_i2c.h"
  #include "Board.h"
#endif
#if PL_CONFIG_HAS_LCD_MENU
  #include "LCDMenu.h"
#endif
#if PL_CONFIG_HAS_EVENTS
  #include "Event.h"
#endif
#if PL_CONFIG_HAS_TRIGGER
  #include "Trigger.h"
#endif
#if PL_CONFIG_HAS_DEBOUNCE
  #include "Debounce.h"
  #include "KeyDebounce.h"
#endif
#if PL_CONFIG_HAS_KEYS
  #include "Keys.h"
#endif
#if PL_CONFIG_HAS_LINE
  #include "Line.h"
#endif
#include "McuHardFault.h"

#if McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT
void McuGenericI2C_CONFIG_ON_ERROR_EVENT(void) {
}
#endif

#if McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT
void McuGenericI2C_CONFIG_ON_REQUEST_BUS_EVENT(void) {
}
#endif

#if McuGenericI2C_CONFIG_USE_ON_RELEASE_BUS_EVENT
void McuGenericI2C_CONFIG_ON_RELEASE_BUS_EVENT(void) {
}
#endif

void PL_Init(void) {
#if PL_CONFIG_USE_FREERTOS
  McuRTOS_Init(); /* must be first to disable the interrupts */
#endif
  BOARD_Init();

  McuHardFault_Init();
  McuWait_Init();
  McuUtility_Init();
  McuShell_Init();
  McuShell_SetStdio(&McuRTT_stdio); /* use RTT for standard I/O */
  McuRTT_Init();
#if PL_CONFIG_HAS_EVENTS
  EVNT_Init();
#endif
#if PL_CONFIG_HAS_TRIGGER
  TRG_Init();
#endif
#if PL_CONFIG_HAS_KEYS
  KEY_Init();
#endif
#if PL_CONFIG_HAS_DEBOUNCE
  DBNC_Init();
  KEYDBNC_Init();
#endif
#if PL_CONFIG_HAS_I2C
  McuGenericI2C_Init();
#endif
#if PL_CONFIG_HAS_SW_I2C
  McuGenericSWI2C_Init();
#elif PL_CONFIG_HAS_HW_I2C
  McuSTM32HALI2C_Init();
  McuSTM32HALI2C_SetDeviceHandle(&hi2c1);
#endif
#if PL_CONFIG_HAS_LCD
  McuSSD1306_Init();
  McuGDisplaySSD1306_Init();
  LCD_Init();
#endif
#if PL_CONFIG_HAS_LCD_MENU
  LCDMenu_Init();
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  McuSystemView_Init();
#elif configUSE_PERCEPIO_TRACE_HOOKS
  //McuPercepio_Init();
#endif
#if PL_CONFIG_HAS_SHELL
  SHELL_Init();
#endif
#if PL_CONFIG_HAS_MOTOR
  PWM_Init();
  MOT_Init();
#endif
#if PL_CONFIG_HAS_PROXIMITY
  PROX_Init();
#endif
#if PL_CONFIG_HAS_QUADRATURE
  QUAD_Init();
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  REF_Init();
#endif
#if PL_CONFIG_HAS_TIMER
  TMR_Init();
#endif
#if PL_CONFIG_HAS_MOTOR_TACHO
  TACHO_Init();
#endif
#if PL_CONFIG_HAS_PID
  PID_Init();
#endif
#if PL_CONFIG_HAS_DRIVE
  DRV_Init();
#endif
#if PL_CONFIG_HAS_TURN
  TURN_Init();
#endif
#if PL_CONFIG_HAS_LINE
  LINE_Init();
#endif
}

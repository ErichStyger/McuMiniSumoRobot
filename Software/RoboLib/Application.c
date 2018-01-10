/**
 * \file
 * \brief Implementation of the RTOS layer.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the RTOS hooks and initialization routines.
 */

#include "Platform.h"
#if PL_CONFIG_USE_FREERTOS
#include "Application.h"
#include "McuRTOS.h"
#include "Led.h"
#include "Pin.h"
#if PL_CONFIG_HAS_SHELL
  #include "Shell.h"
  #include "McuShell.h"
#endif
#if PL_CONFIG_HAS_DEBOUNCE
  #include "KeyDebounce.h"
#endif
#if PL_CONFIG_HAS_KEYS
  #include "Keys.h"
#endif
#if PL_CONFIG_HAS_MOTOR
  #include "Motor.h"
  #include "PWM.h"
#endif
#if PL_CONFIG_HAS_DRIVE
  #include "Drive.h"
#endif
#if PL_CONFIG_HAS_SUMO
  #include "Sumo.h"
#endif


#if PL_CONFIG_USE_FREERTOS

#if !PL_CONFIG_HAS_DEBOUNCE
static bool ButtonPressed(void) {
  if (PIN_IsPinHigh(PIN_SW3)) { /* User button pressed? */
	  vTaskDelay(pdMS_TO_TICKS(50)); /* simple debounce */
	  if (PIN_IsPinHigh(PIN_SW3)) { /* still pressed? */
		  return TRUE;
	  }
  }
  return FALSE;
}
#endif

static void AppTask(void *pvParameters) {
#if PL_CONFIG_DO_TEST_IR
	bool doTracking = FALSE;
#endif
  (void)pvParameters; /* parameter not used */
  for(;;) {
#if PL_CONFIG_HAS_DEBOUNCE
    KEYDBNC_Process();
#else
    KEY_Scan(); /* scan keys and set events */
#endif
#if PL_CONFIG_DO_TEST_PUSH
	  if (ButtonPressed()) { /* User button pressed? */
		vTaskDelay(pdMS_TO_TICKS(2000)); /* wait 2 seconds */
		MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), 100);
		MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), 100);
		vTaskDelay(pdMS_TO_TICKS(3000)); /* run for some time */
		MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), 0);
		MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), 0);
	  }
#elif PL_CONFIG_DO_TEST_IR
	  if (ButtonPressed()) { /* User button pressed? */
		  doTracking = !doTracking;
		  if (doTracking) {
		    McuShell_printf("Starting tracking...\r\n");
		  } else {
			McuShell_printf("Tracking stopped!\r\n");
		  }
		  vTaskDelay(pdMS_TO_TICKS(2000)); /* wait some time */
	  }
	  if (doTracking) {
		  if (PROX_HasTarget()) {
			  int angle = PROX_GetTargetAngle();

			  if  (angle<0) {
				  McuShell_printf("Target on the left\r\n");
				  TURN_TurnAngle(-3, NULL);
				  vTaskDelay(pdMS_TO_TICKS(100)); /* wait some time */
			  } else if(angle>0) {
				  McuShell_printf("Target on the right\r\n");
				  TURN_TurnAngle(3, NULL);
				  vTaskDelay(pdMS_TO_TICKS(100)); /* wait some time */
			  }
		  }
	  }
#endif
	  vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void APP_Run(void) {
  PL_Init();
#if PL_CONFIG_HAS_SUMO
  SUMO_Init();
#endif
  if (xTaskCreate(
		AppTask,  /* pointer to the task */
        "AppTask", /* task name for kernel awareness debugging */
        250/sizeof(StackType_t), /* task stack size */
        (void*)NULL, /* optional task startup argument */
        tskIDLE_PRIORITY+1,  /* initial priority */
        (xTaskHandle*)NULL /* optional task handle to create */
      ) != pdPASS) {
    /*lint -e527 */
    for(;;){}; /* error! probably out of memory */
    /*lint +e527 */
  }
  vTaskStartScheduler();
}
#endif

#endif /* PL_CONFIG_USE_FREERTOS */

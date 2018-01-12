/*
 * Sumo.c
 *
 *  Created on: 19.12.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#if PL_CONFIG_HAS_SUMO
#include "Sumo.h"
#include "McuShell.h"
#include "McuUtility.h"
#include "Motor.h"
#include "Proximity.h"
#include "Pin.h"
#include "Drive.h"
#include "Turn.h"
#include "Reflectance.h"

#define SUMO_DRIVE_SPEED   (1000)
#define SUMO_CHASE_SPEED   (2000)
#define SUMO_USE_PROXY     (0 && PL_CONFIG_HAS_PROXIMITY)

/* direct task notification bits */
#define SUMO_START_SUMO (1<<0)  /* start sumo mode */
#define SUMO_STOP_SUMO  (1<<1)  /* stop stop sumo */
static TaskHandle_t sumoTaskHndl;
static int16_t sumoCntDownMs = 0;

typedef enum {
	SUMO_STATE_IDLE,
  SUMO_STATE_COUNTDOWN,
	SUMO_STATE_START_RUNNING,
	SUMO_STATE_RUNNING,
	SUMO_STATE_STOP,
} SUMO_State_t;

static SUMO_State_t SUMO_state;

static bool ButtonPressed(void) {
#if !PL_CONFIG_HAS_LCD_MENU /* sumo gets started and stopped through LCD menu */
  return EVNT_EventIsSetAutoClear(EVNT_SW1_RELEASED);
#else
  uint32_t notifcationValue;
  (void)xTaskNotifyWait(0UL, SUMO_START_SUMO|SUMO_STOP_SUMO, &notifcationValue, 0); /* check flags */
  return notifcationValue!=0;
#endif
}

bool SUMO_IsDoingSumo(void) {
  return SUMO_state!=SUMO_STATE_IDLE;
}

void SUMO_StartSumo(void) {
  (void)xTaskNotify(sumoTaskHndl, SUMO_START_SUMO, eSetBits);
}

void SUMO_StopSumo(void) {
  (void)xTaskNotify(sumoTaskHndl, SUMO_STOP_SUMO, eSetBits);
}

void SUMO_StartStopSumo(void) {
  if (SUMO_IsDoingSumo()) {
    SUMO_StopSumo();
  } else {
    SUMO_StartSumo();
  }
}

int16_t SUMO_GetCountDownMs(void) {
  return sumoCntDownMs;
}

static void SumoStateMachine(void) {
	uint32_t refVal;
	uint8_t bits;

	for(;;) { /* breaks */
		switch(SUMO_state) {
			case SUMO_STATE_IDLE:
				if (ButtonPressed()) {
					refVal = REF_IsWhite();
					if (refVal==0) { /* all black */
					  SUMO_state = SUMO_STATE_COUNTDOWN;
					  continue;
					}
				}
				break;

			case SUMO_STATE_COUNTDOWN:
			  sumoCntDownMs = 5000;
			  while(sumoCntDownMs>0) { /* 5 seconds delay */
			    vTaskDelay(pdMS_TO_TICKS(50));
			    sumoCntDownMs -= 50;
			    if (ButtonPressed()) {
			      break; /* abort */
			    }
			  }
			  if (sumoCntDownMs<=0) { /* count down expired */
			    SUMO_state = SUMO_STATE_START_RUNNING;
			  } else {
          SUMO_state = SUMO_STATE_IDLE; /* aborted */
			  }
        sumoCntDownMs = 0;
			  break;

			case SUMO_STATE_START_RUNNING:
				DRV_SetSpeed(SUMO_DRIVE_SPEED, SUMO_DRIVE_SPEED);
				DRV_SetMode(DRV_MODE_SPEED);
				SUMO_state = SUMO_STATE_RUNNING;
				continue; /* advance to next state */
			case SUMO_STATE_RUNNING:
				if (ButtonPressed()) {
					SUMO_state = SUMO_STATE_STOP;
					continue; /* advance to next state */
				}
				refVal = REF_IsWhite();
				if (refVal!=0) { /* white? */
				  TURN_Turn(TURN_STEP_BORDER_BW, NULL);
				  if (refVal&0x3) {
					TURN_Turn(TURN_RIGHT90, NULL);
				  } else {
				    TURN_Turn(TURN_LEFT90, NULL);
				  }
				  SUMO_state = SUMO_STATE_START_RUNNING;
				  continue; /* advance to next state */
				}
#if SUMO_USE_PROXY
				if (PROX_HasTarget()) {
					int angle = PROX_GetTargetAngle();
					if (angle==0) { /* in front */
						DRV_SetSpeed(SUMO_CHASE_SPEED, SUMO_CHASE_SPEED);
					} else if (angle>=-90 && angle<=90) {
						TURN_TurnAngle(angle, NULL);
						SUMO_state = SUMO_STATE_START_RUNNING;
						continue; /* advance to next state */
					}
				}
#endif
				break;
			case SUMO_STATE_STOP:
				DRV_SetMode(DRV_MODE_STOP);
				SUMO_state = SUMO_STATE_IDLE;
				while (ButtonPressed()) {
					vTaskDelay(pdMS_TO_TICKS(50));
				}
				break;
			default:
				break;
		} /* switch */
		break; /* leave for() loop */
	} /* for */
}

static void SumoTask(void *pvParameters) {
  (void)pvParameters; /* parameter not used */

  SUMO_state = SUMO_STATE_IDLE;
  for(;;) {
	  SumoStateMachine();
	  vTaskDelay(pdMS_TO_TICKS(5));
  }
}

#if PL_CONFIG_HAS_SHELL
uint8_t SUMO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res=ERR_OK;

  if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "sumo help")==0) {
    McuShell_SendHelpStr((const unsigned char*)"sumo", (const unsigned char*)"Sumo command group\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "sumo status")==0) {
    McuShell_SendStr((const unsigned char*)"sumo:\r\n", io->stdOut);
    McuShell_SendStatusStr((const unsigned char*)"  tbd", (const unsigned char*)"", io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);
    *handled = TRUE;
  }
  return res;
}
#endif /* PL_CONFIG_HAS_SHELL */


void SUMO_Init(void) {
  if (xTaskCreate(
    SumoTask,  /* pointer to the task */
        "SumoTask", /* task name for kernel awareness debugging */
        300/sizeof(StackType_t), /* task stack size */
        (void*)NULL, /* optional task startup argument */
        tskIDLE_PRIORITY+1,  /* initial priority */
        &sumoTaskHndl /* optional task handle to create */
      ) != pdPASS) {
    /*lint -e527 */
    for(;;){}; /* error! probably out of memory */
    /*lint +e527 */
  }
}
#endif /* PL_CONFIG_HAS_SUMO */

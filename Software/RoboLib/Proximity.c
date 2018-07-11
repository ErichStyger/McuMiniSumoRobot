/**
 * \file
 * \brief Proximity Module
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module handles the proximity (IR) sensors to detect an obstacle or another robot.
 */

#include "McuLib.h"
#include "Proximity.h"
#include "McuUtility.h"
#include "Pin.h"

#define PROX_NOF_LEVELS    5
static const int PROX_durationBurstUs[PROX_NOF_LEVELS] = {50, 100, 200, 350, 500};

static struct {
  bool proximityFound;
  PROX_Bits proximityBits;
  int proximityAngle;
  uint8_t countsLeft[PROX_NOF_SENSORS];
  uint8_t countsRight[PROX_NOF_SENSORS];
} PROX_status;

static const int16_t PROX_Angles[] = {
		      /* LL LM LR RL RM RR */
	360,	  /* 0  0  0  0  0  0  */
	 90,	  /* 0  0  0  0  0  1  */
	 10,	  /* 0  0  0  0  1  0  */
	 45,	  /* 0  0  0  0  1  1  */
	-10,	  /* 0  1  0  0  0  0  */
	  0,    /* 0  1  0  0  0  1  */
	  0,    /* 0  1  0  0  1  0  */
	 30,    /* 0  1  0  0  1  1  */
	-90,    /* 1  0  0  0  0  0  */
	360,    /* 1  0  0  0  0  1  */
	-30,    /* 1  0  0  0  1  0  */
	 45,    /* 1  0  0  0  1  1  */
	-45,    /* 1  1  0  0  0  0  */
	-45,    /* 1  1  0  0  0  1  */
	-30,    /* 1  1  0  0  1  0  */
	  0,    /* 1  1  0  0  1  1  */
};

static void CheckProx(bool isLeft, PROX_Bits *proxBits) {
	/* Signal is active LOW: low means we have a reflection */
	if (isLeft) {
		if (PIN_IsPinLow(PIN_PROX_L)) {
			*proxBits |= PROX_L_LEFT_BIT;
		}
		if (PIN_IsPinLow(PIN_PROX_M)) {
			*proxBits |= PROX_L_MIDDLE_BIT;
		}
    if (PIN_IsPinLow(PIN_PROX_R)) {
      *proxBits |= PROX_L_RIGHT_BIT;
    }
	} else {
    if (PIN_IsPinLow(PIN_PROX_L)) {
      *proxBits |= PROX_R_LEFT_BIT;
    }
		if (PIN_IsPinLow(PIN_PROX_M)) {
			*proxBits |= PROX_R_MIDDLE_BIT;
		}
		if (PIN_IsPinLow(PIN_PROX_R)) {
			*proxBits |= PROX_R_RIGHT_BIT;
		}
	}
}

uint8_t PROX_GetNofWithLeftLeds(uint8_t sensorIdx) {
  if (sensorIdx>=PROX_NOF_SENSORS) {
    return 0; /* error */
  }
  return PROX_status.countsLeft[sensorIdx];
}

uint8_t PROX_GetNofWithRightLeds(uint8_t sensorIdx) {
  if (sensorIdx>=PROX_NOF_SENSORS) {
    return 0; /* error */
  }
  return PROX_status.countsRight[sensorIdx];
}

static bool PROX_CheckProximity(int *pAngle, uint8_t *pBits) {
	uint8_t bits;
  int i;

  for(i=0;i<PROX_NOF_SENSORS;i++) { /* init */
    PROX_status.countsLeft[i] = 0;
    PROX_status.countsRight[i] = 0;
  }

  /* check left side */
  for(i=0;i<PROX_NOF_LEVELS;i++) {
    PIN_SetHigh(PIN_PROX_IR_SELECT); /* HIGH: select left IR sender */
    McuWait_Waitus(PROX_durationBurstUs[i]);
    if (PIN_IsPinLow(PIN_PROX_L)) {
      PROX_status.countsLeft[0]++;
    }
    if (PIN_IsPinLow(PIN_PROX_M)) {
      PROX_status.countsLeft[1]++;
    }
    if (PIN_IsPinLow(PIN_PROX_R)) {
      PROX_status.countsLeft[2]++;
    }
    PIN_SetLow(PIN_PROX_IR_SELECT); /* LOW: select Right IR sender */
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  /* check right side */
  for(i=0;i<PROX_NOF_LEVELS;i++) {
    PIN_SetLow(PIN_PROX_IR_SELECT); /* HIGH: select left IR sender */
    McuWait_Waitus(PROX_durationBurstUs[i]);
    if (PIN_IsPinLow(PIN_PROX_L)) {
      PROX_status.countsRight[0]++;
    }
    if (PIN_IsPinLow(PIN_PROX_M)) {
      PROX_status.countsRight[1]++;
    }
    if (PIN_IsPinLow(PIN_PROX_R)) {
      PROX_status.countsRight[2]++;
    }
    PIN_SetHigh(PIN_PROX_IR_SELECT); /* High: select left IR sender */
    vTaskDelay(pdMS_TO_TICKS(1));
  }

	*pBits = 0;
#if 1
	PIN_SetHigh(PIN_PROX_IR_SELECT); /* HIGH: select left IR sender */
	vTaskDelay(pdMS_TO_TICKS(5)); /* give the LED some time */
	CheckProx(true, pBits);
#endif
	PIN_SetLow(PIN_PROX_IR_SELECT); /* LOW: select right IR sender */
	vTaskDelay(pdMS_TO_TICKS(5)); /* give the LED some time */
	CheckProx(false, pBits);
	bits = *pBits;
#if 1 /* ignore the 'strange' bits */
	bits = ((bits&(PROX_L_LEFT_BIT|PROX_L_MIDDLE_BIT))>>2) | (bits&(PROX_R_MIDDLE_BIT|PROX_R_RIGHT_BIT));
#endif
	if (bits<sizeof(PROX_Angles)/sizeof(PROX_Angles[0])) {
		*pAngle = PROX_Angles[bits];
		return (*pAngle!=360);
	} else {
		return FALSE;
	}
}

bool PROX_HasTarget(void) {
	return PROX_status.proximityFound;
}

PROX_Bits PROX_GetProxBits(void) {
	return PROX_status.proximityBits;
}

int PROX_GetTargetAngle(void) {
	return PROX_status.proximityAngle;
}

#if PL_CONFIG_HAS_SHELL
uint8_t PROX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res=ERR_OK;
  uint8_t buf[24];

  if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "prox help")==0) {
    McuShell_SendHelpStr((const unsigned char*)"prox", (const unsigned char*)"Prox command group\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  reset", (const unsigned char*)"Reset the current position counter\r\n", io->stdOut);
    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "prox status")==0) {
    McuShell_SendStr((const unsigned char*)"  prox:\r\n", io->stdOut);
    McuShell_SendStatusStr((unsigned char*)"  target:", PROX_HasTarget()?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
    McuUtility_strcpy(buf, sizeof(buf), (PROX_status.proximityBits&PROX_L_LEFT_BIT)?(uint8_t*)"L":(uint8_t*)".");
   	McuUtility_strcat(buf, sizeof(buf), (PROX_status.proximityBits&PROX_L_MIDDLE_BIT)?(uint8_t*)"M|":(uint8_t*)".|");
   	McuUtility_strcat(buf, sizeof(buf), (PROX_status.proximityBits&PROX_R_MIDDLE_BIT)?(uint8_t*)"M":(uint8_t*)".");
   	McuUtility_strcat(buf, sizeof(buf), (PROX_status.proximityBits&PROX_R_RIGHT_BIT)?(uint8_t*)"R":(uint8_t*)".");
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\r\n");
    McuShell_SendStatusStr((unsigned char*)"  IR:", buf, io->stdOut);

    McuUtility_Num16sToStr(buf, sizeof(buf), PROX_status.proximityAngle);
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\r\n");
    McuShell_SendStatusStr((unsigned char*)"  angle:", buf, io->stdOut);
    *handled = TRUE;
  }
  return res;
}
#endif /* PL_CONFIG_HAS_SHELL */

static void ProxTask(void *pvParameters) {
  int i, angle = 0;
  uint8_t bits = 0;

  (void)pvParameters; /* parameter not used */
  PROX_status.proximityBits = 0;
  PROX_status.proximityAngle = 0;
  PROX_status.proximityFound = FALSE;
  for(i=0;i<PROX_NOF_SENSORS;i++) { /* init */
    PROX_status.countsLeft[i] = 0;
    PROX_status.countsRight[i] = 0;
  }
  for(;;) {
	  PROX_status.proximityFound = PROX_CheckProximity(&angle, &bits);
	  PROX_status.proximityAngle = angle;
	  PROX_status.proximityBits = bits;
	  vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void PROX_Init(void) {
  if (xTaskCreate(
		ProxTask,  /* pointer to the task */
		"ProxTask", /* task name for kernel awareness debugging */
		(200+100)/sizeof(StackType_t), /* task stack size */
		(void*)NULL, /* optional task startup argument */
		tskIDLE_PRIORITY+2,  /* initial priority */
		(xTaskHandle*)NULL /* optional task handle to create */
	  ) != pdPASS) {
	/*lint -e527 */
	for(;;){}; /* error! probably out of memory */
	/*lint +e527 */
  }
}

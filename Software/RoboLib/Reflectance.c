/*
 * Reflectance.c
 *
 *  Created on: 15.12.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "Reflectance.h"
#include "McuShell.h"
#include "McuUtility.h"
#include "McuWait.h"
#include "Pin.h"
#include "Timer.h"
#if PL_CONFIG_HAS_LINE
  #include "Line.h"
#endif

#define REF_SENSOR_TIMEOUT_US  1000   /* after this time, consider no reflection (black). Must be smaller than the timeout period of the RefCnt timer! */
#define REF_TIMEOUT_TICKS      0xa000

static REF_SensorTimeType SensorRaw[REF_NOF_SENSORS]; /* raw sensor values */

#define EDGE_L_Get()     (HAL_GPIO_ReadPin(PIN_EDGE_L_PORT, PIN_EDGE_L_PIN)==GPIO_PIN_SET)
#define EDGE_ML_Get()    (HAL_GPIO_ReadPin(PIN_EDGE_ML_PORT, PIN_EDGE_ML_PIN)==GPIO_PIN_SET)
#define EDGE_MR_Get()    (HAL_GPIO_ReadPin(PIN_EDGE_MR_PORT, PIN_EDGE_MR_PIN)==GPIO_PIN_SET)
#define EDGE_R_Get()     (HAL_GPIO_ReadPin(PIN_EDGE_R_PORT, PIN_EDGE_R_PIN)==GPIO_PIN_SET)

REF_SensorTimeType REF_GetRawValue(unsigned int idx) {
  if (idx<REF_NOF_SENSORS) {
    return SensorRaw[idx];
  }
  return 0; /* error case */
}

static void SetOutputHigh(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Alternate = 0; /* init to default value */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* configure as push/pull output pin */
  GPIO_InitStruct.Pull = GPIO_NOPULL; /* no pull-up needed */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_WritePin(PIN_EDGE_L_PORT, PIN_EDGE_L_PIN, GPIO_PIN_SET); /* HIGH */
  GPIO_InitStruct.Pin = PIN_EDGE_L_PIN; HAL_GPIO_Init(PIN_EDGE_L_PORT, &GPIO_InitStruct); /* output */

  HAL_GPIO_WritePin(PIN_EDGE_ML_PORT, PIN_EDGE_ML_PIN, GPIO_PIN_SET); /* HIGH */
  GPIO_InitStruct.Pin = PIN_EDGE_ML_PIN; HAL_GPIO_Init(PIN_EDGE_ML_PORT, &GPIO_InitStruct); /* output */

  HAL_GPIO_WritePin(PIN_EDGE_MR_PORT, PIN_EDGE_MR_PIN, GPIO_PIN_SET); /* HIGH */
  GPIO_InitStruct.Pin = PIN_EDGE_MR_PIN; HAL_GPIO_Init(PIN_EDGE_MR_PORT, &GPIO_InitStruct); /* output */

  HAL_GPIO_WritePin(PIN_EDGE_R_PORT, PIN_EDGE_R_PIN, GPIO_PIN_SET); /* HIGH */
  GPIO_InitStruct.Pin = PIN_EDGE_R_PIN; HAL_GPIO_Init(PIN_EDGE_R_PORT, &GPIO_InitStruct); /* output */
}

static void SetInput(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Alternate = 0; /* init to default value */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; /* configure as push/pull output pin */
  GPIO_InitStruct.Pull = GPIO_NOPULL; /* no pull-up needed */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = PIN_EDGE_L_PIN; HAL_GPIO_Init(PIN_EDGE_L_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = PIN_EDGE_ML_PIN; HAL_GPIO_Init(PIN_EDGE_ML_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = PIN_EDGE_MR_PIN; HAL_GPIO_Init(PIN_EDGE_MR_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = PIN_EDGE_R_PIN; HAL_GPIO_Init(PIN_EDGE_R_PORT, &GPIO_InitStruct);
}

uint32_t REF_IsWhite(void) {
	uint32_t val = 0;
	uint32_t mask = 1;
	int i;

	taskENTER_CRITICAL();
	for(i=0;i<REF_NOF_SENSORS;i++) {
		if (SensorRaw[i] <= 0x8000/*White! REF_MIN_LINE_VAL*/){
			val |= mask;
		}
		mask <<= 1;
	}
	taskEXIT_CRITICAL();
	return val;
}

static void REF_MeasureRaw(void) {
	int i;
	uint32_t timerValue;

	for(i=0;i<REF_NOF_SENSORS;i++) {
		SensorRaw[i] = REF_MAX_SENSOR_VALUE; /* init with 0xffff'ffff */
	}
	SetOutputHigh();
	McuWait_Waitus(20); /* give time to charge */
	TMRR_SetCounter(0); /* reset timer */
	taskENTER_CRITICAL();
	TMRR_Start(); /* start timer */
	SetInput();
	for(;;) { /* breaks */
		timerValue = TMRR_GetCounter();
		if (timerValue>REF_TIMEOUT_TICKS) {
#if 0
		  for(i=0;i<REF_NOF_SENSORS;i++) {
		    if (SensorRaw[i]==REF_MAX_SENSOR_VALUE) { /* not measured to LOW */
		      SensorRaw[i] = REF_TIMEOUT_TICKS; /* set to timeout value */
		    }
		  }
#endif
			break; /* timeout */
		}
		if (SensorRaw[0]==REF_MAX_SENSOR_VALUE && EDGE_L_Get()==0) { /* discharged to low */
			SensorRaw[0] = timerValue;
		}
		if (SensorRaw[1]==REF_MAX_SENSOR_VALUE && EDGE_ML_Get()==0) { /* discharged to low */
			SensorRaw[1] = timerValue;
		}
		if (SensorRaw[2]==REF_MAX_SENSOR_VALUE && EDGE_MR_Get()==0) { /* discharged to low */
			SensorRaw[2] = timerValue;
		}
		if (SensorRaw[3]==REF_MAX_SENSOR_VALUE && EDGE_R_Get()==0) { /* discharged to low */
			SensorRaw[3] = timerValue;
		}
		if (       SensorRaw[0]!=REF_MAX_SENSOR_VALUE
				&& SensorRaw[1]!=REF_MAX_SENSOR_VALUE
				&& SensorRaw[2]!=REF_MAX_SENSOR_VALUE
				&& SensorRaw[3]!=REF_MAX_SENSOR_VALUE
				)
		{
			break; /* all sensors measured */
		}
	} /* for */
	taskEXIT_CRITICAL();
	TMRR_Stop();
}

static void RefTask(void *pvParameters) {
  (void)pvParameters; /* parameter not used */
  for(;;) {
    REF_MeasureRaw();
  #if PL_CONFIG_HAS_LINE
    LINE_StateMachine();
  #endif
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

#if PL_CONFIG_HAS_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];
  int i;

  McuShell_SendStatusStr((unsigned char*)"reflectance", (unsigned char*)"\r\n", io->stdOut);

  McuUtility_Num16uToStr(buf, sizeof(buf), REF_SENSOR_TIMEOUT_US);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" us, 0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_TIMEOUT_TICKS);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ticks\r\n");
  McuShell_SendStatusStr((unsigned char*)"  timeout", buf, io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  raw val", (unsigned char*)"", io->stdOut);
  for (i=0;i<REF_NOF_SENSORS;i++) {
    if (i==0) {
      McuShell_SendStr((unsigned char*)"0x", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
    }
    buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorRaw[i]);
    McuShell_SendStr(buf, io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  return ERR_OK;
}

uint8_t REF_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
	  uint8_t res=ERR_OK;
	  uint8_t buf[24];

	  if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "ref help")==0) {
	    McuShell_SendHelpStr((const unsigned char*)"ref", (const unsigned char*)"Reflectance command group\r\n", io->stdOut);
	    McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
	    *handled = TRUE;
	  } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "ref status")==0) {
	    *handled = TRUE;
	    return PrintStatus(io);
	  }
	  return res;
}
#endif

void REF_Init(void) {
  if (xTaskCreate(
		RefTask,  /* pointer to the task */
		"RefTask", /* task name for kernel awareness debugging */
		400/sizeof(StackType_t), /* task stack size */
		(void*)NULL, /* optional task startup argument */
		tskIDLE_PRIORITY+2,  /* initial priority */
		(xTaskHandle*)NULL /* optional task handle to create */
	  ) != pdPASS) {
	/*lint -e527 */
	for(;;){}; /* error! probably out of memory */
	/*lint +e527 */
  }
}

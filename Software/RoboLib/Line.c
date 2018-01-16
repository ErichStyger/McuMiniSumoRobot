/**
 * \file
 * \brief Line sensor
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the driver a line sensor based on the IR reflectance sensor.
 */

#include "Platform.h"
#if PL_CONFIG_HAS_LINE
#include "Reflectance.h"

#define LINE_USE_WHITE_LINE     (0)
#define LINE_MIN_NOISE_VAL      0x20   /* values below this are not added to the weighted sum */

static uint16_t LINE_linePos = 0;
static xSemaphoreHandle LINE_StartStopCalibSem = NULL;

typedef enum {
  LINE_STATE_INIT,
  LINE_STATE_NOT_CALIBRATED,
  LINE_STATE_START_CALIBRATION,
  LINE_STATE_CALIBRATING,
  LINE_STATE_STOP_CALIBRATION,
  LINE_STATE_SAVE_CALIBRATION,
  LINE_STATE_READY
} lineStateType;

static volatile lineStateType lineState = LINE_STATE_INIT;

typedef struct {
  REF_SensorTimeType minVal[REF_NOF_SENSORS];
  REF_SensorTimeType maxVal[REF_NOF_SENSORS];
  REF_SensorTimeType oneKVal[REF_NOF_SENSORS]; /* 0 means white/min value, 1000 means black/max value */
} REF_SensorVal_t;

static REF_SensorVal_t   SensorValues; /* values from calibration */

REF_SensorTimeType LINE_Get1kValue(unsigned int idx) {
  if (idx<REF_NOF_SENSORS) {
    return SensorValues.oneKVal[idx];
  }
  return 0; /* error case */
}

REF_SensorTimeType LINE_GetMinValue(unsigned int idx) {
  if (idx<REF_NOF_SENSORS) {
    return SensorValues.minVal[idx];
  }
  return 0; /* error case */
}
REF_SensorTimeType LINE_GetMaxValue(unsigned int idx) {
  if (idx<REF_NOF_SENSORS) {
    return SensorValues.maxVal[idx];
  }
  return 0; /* error case */
}

/*
 * Operates the same as read calibrated, but also returns an
 * estimated position of the robot with respect to a line. The
 * estimate is made using a weighted average of the sensor indices
 * multiplied by 1000, so that a return value of 1000 indicates that
 * the line is directly below sensor 0, a return value of 2000
 * indicates that the line is directly below sensor 1, 2000
 * indicates that it's below sensor 2000, etc. Intermediate
 * values indicate that the line is between two sensors. The
 * formula is:
 *
 * 1000*value0 + 2000*value1 + 3000*value2 + ...
 * --------------------------------------------
 * value0 + value1 + value2 + ...
 *
 * By default, this function assumes a dark line (high values)
 * surrounded by white (low values). If your line is light on
 * black, set the optional second argument white_line to true. In
 * this case, each sensor value will be replaced by (1000-value)
 * before the averaging.
 */
static uint16_t ReadLine(bool white_line) {
  int i;
  unsigned long avg; /* this is for the weighted total, which is long */
  /* before division */
  unsigned int sum; /* this is for the denominator which is <= 64000 */
  unsigned int mul; /* multiplication factor, 0, 1000, 2000, 3000 ... */
  int value;

  avg = 0;
  sum = 0;
  mul = 1000;
  for(i=0;i<REF_NOF_SENSORS;i++) {
    value = SensorValues.oneKVal[i];
    if(white_line) {
      value = 1000-value;
    }
    /* only average in values that are above a noise threshold */
    if(value > LINE_MIN_NOISE_VAL) {
      avg += ((long)value)*mul;
      sum += value;
    }
    mul += 1000;
  }
  return avg/sum;
}

static void MeasureRawMinMax(void) {
 int i;
 REF_SensorTimeType val;

 for(i=0;i<REF_NOF_SENSORS;i++) {
   val = REF_GetRawValue(i);
   if (val < SensorValues.minVal[i]) {
     SensorValues.minVal[i] = val;
   }
   if (val > SensorValues.maxVal[i]) {
     SensorValues.maxVal[i] = val;
   }
 }
}

static void Calc1kValues(void) {
 int i;
 int32_t x, denominator;

 for(i=0;i<REF_NOF_SENSORS;i++) {
   x = 0;
   denominator = SensorValues.maxVal[i]-SensorValues.minVal[i];
   if (denominator!=0) {
     x = (((int32_t)REF_GetRawValue(i)-SensorValues.minVal[i])*1000)/denominator;
   }
   if (x<0) {
     x = 0;
   } else if (x>1000) {
     x = 1000;
   }
   SensorValues.oneKVal[i] = x;
 }
}

void LINE_CalibrateStartStop(void) {
 if (lineState==LINE_STATE_NOT_CALIBRATED || lineState==LINE_STATE_CALIBRATING || lineState==LINE_STATE_READY) {
   (void)xSemaphoreGive(LINE_StartStopCalibSem);
 }
}

bool LINE_IsCalibrating(void) {
  return lineState==LINE_STATE_CALIBRATING || lineState==LINE_STATE_START_CALIBRATION;
}

uint16_t LINE_GetLinePos(void) {
  /* 0 means no line, >0 means line is below sensor 0, 1000 below sensor 1 and so on */
  return LINE_linePos;
}

static void LINE_CalcLineValue(void) {
  Calc1kValues();
  LINE_linePos = ReadLine(LINE_USE_WHITE_LINE);
}

void LINE_StateMachine(void) {
  int i;

  switch (lineState) {
    case LINE_STATE_INIT:
    #if PL_CONFIG_HAS_CONFIG_NVM
    {
      SensorCalibT *ptr;

      ptr = (SensorCalibT*)NVMC_GetReflectanceData();
      if (ptr!=NULL) { /* valid data */
        SensorCalibMinMax = *ptr; /* struct copy */
        lineState = LINE_STATE_READY;
      } else {
        lineState = LINE_STATE_NOT_CALIBRATED;
      }
    }
    #else
      McuShell_SendStr((unsigned char*)"INFO: No calibration data present.\r\n", McuShell_GetStdio()->stdOut);
      lineState = LINE_STATE_NOT_CALIBRATED;
    #endif
      break;

    case LINE_STATE_NOT_CALIBRATED:
      if (xSemaphoreTake(LINE_StartStopCalibSem, 0)==pdTRUE) {
        lineState = LINE_STATE_START_CALIBRATION;
      }
      break;

    case LINE_STATE_START_CALIBRATION:
      McuShell_SendStr((unsigned char*)"starting calibration...\r\n", McuShell_GetStdio()->stdOut);
      for(i=0;i<REF_NOF_SENSORS;i++) {
        SensorValues.minVal[i] = REF_MAX_SENSOR_VALUE;
        SensorValues.maxVal[i] = 0;
        SensorValues.oneKVal[i] = 0;
      }
      lineState = LINE_STATE_CALIBRATING;
      break;

    case LINE_STATE_CALIBRATING:
      MeasureRawMinMax();
      LINE_CalcLineValue();
      if (xSemaphoreTake(LINE_StartStopCalibSem, 0)==pdTRUE) {
        lineState = LINE_STATE_STOP_CALIBRATION;
      }
      break;

    case LINE_STATE_STOP_CALIBRATION:
      McuShell_SendStr((unsigned char*)"...stopped calibration.\r\n", McuShell_GetStdio()->stdOut);
#if PL_CONFIG_HAS_CONFIG_NVM
      if (NVMC_SaveReflectanceData(&SensorCalibMinMax, sizeof(SensorCalibMinMax))!=ERR_OK) {
        SHELL_SendString((unsigned char*)"Flashing calibration data FAILED!\r\n");
      } else {
        SHELL_SendString((unsigned char*)"Stored calibration data.\r\n");
      }
#endif
      lineState = LINE_STATE_READY;
      break;

    case LINE_STATE_READY:
      LINE_CalcLineValue();
      if (xSemaphoreTake(LINE_StartStopCalibSem, 0)==pdTRUE) {
        lineState = LINE_STATE_START_CALIBRATION;
      }
      break;
  } /* switch */
}

#if PL_CONFIG_HAS_SHELL
static unsigned char*GetStateString(void) {
  switch (lineState) {
    case LINE_STATE_INIT:                return (unsigned char*)"INIT";
    case LINE_STATE_NOT_CALIBRATED:      return (unsigned char*)"NOT CALIBRATED";
    case LINE_STATE_START_CALIBRATION:   return (unsigned char*)"START CALIBRATION";
    case LINE_STATE_CALIBRATING:         return (unsigned char*)"CALIBRATING";
    case LINE_STATE_STOP_CALIBRATION:    return (unsigned char*)"STOP CALIBRATION";
    case LINE_STATE_SAVE_CALIBRATION:    return (unsigned char*)"SAVE CALIBRATION";
    case LINE_STATE_READY:               return (unsigned char*)"READY";
    default:
      break;
  } /* switch */
  return (unsigned char*)"UNKNOWN";
}
#endif

#if PL_CONFIG_HAS_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];
  int i;

  McuShell_SendStatusStr((unsigned char*)"line", (unsigned char*)"\r\n", io->stdOut);
#if LINE_USE_WHITE_LINE
  McuShell_SendStatusStr((unsigned char*)"  line mode", (unsigned char*)"white\r\n", io->stdOut);
#else
  McuShell_SendStatusStr((unsigned char*)"  line mode", (unsigned char*)"black\r\n", io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  calib min", (unsigned char*)"", io->stdOut);
  for (i=0;i<REF_NOF_SENSORS;i++) {
    if (i==0) {
      McuShell_SendStr((unsigned char*)"0x", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
    }
    buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorValues.minVal[i]);
    McuShell_SendStr(buf, io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  calib max", (unsigned char*)"", io->stdOut);
  for (i=REF_NOF_SENSORS-1;i>=0;i--) {
    if (i==REF_NOF_SENSORS-1) {
      McuShell_SendStr((unsigned char*)"0x", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
    }
    buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorValues.maxVal[i]);
    McuShell_SendStr(buf, io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  1k val", (unsigned char*)"", io->stdOut);
  for (i=0;i<REF_NOF_SENSORS;i++) {
    if (i==0) {
      McuShell_SendStr((unsigned char*)"0x", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
    }
    buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorValues.oneKVal[i]);
    McuShell_SendStr(buf, io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  state", GetStateString(), io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), LINE_MIN_NOISE_VAL);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  min noise", buf, io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  line pos", (unsigned char*)"", io->stdOut);
  buf[0] = '\0'; McuUtility_strcatNum16s(buf, sizeof(buf), LINE_linePos);
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  return ERR_OK;
}

uint8_t LINE_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
    uint8_t res=ERR_OK;
    uint8_t buf[24];

    if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "line help")==0) {
      McuShell_SendHelpStr((const unsigned char*)"line", (const unsigned char*)"Line command group\r\n", io->stdOut);
      McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
      McuShell_SendHelpStr((const unsigned char*)"  calib (start|stop)", (unsigned char*)"Start/Stop calibration\r\n", io->stdOut);
      *handled = TRUE;
    } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "line status")==0) {
      *handled = TRUE;
      return PrintStatus(io);
    } else if (McuUtility_strcmp((char*)cmd, "line calib start")==0) {
      if (lineState==LINE_STATE_NOT_CALIBRATED || lineState==LINE_STATE_READY) {
        LINE_CalibrateStartStop();
      } else {
        McuShell_SendStr((unsigned char*)"ERROR: cannot start calibration, must not be calibrating or be ready.\r\n", io->stdErr);
        return ERR_FAILED;
      }
      *handled = TRUE;
      return ERR_OK;
    } else if (McuUtility_strcmp((char*)cmd, "line calib stop")==0) {
      if (lineState==LINE_STATE_CALIBRATING) {
        LINE_CalibrateStartStop();
      } else {
        McuShell_SendStr((unsigned char*)"ERROR: can only stop if calibrating.\r\n", io->stdErr);
        return ERR_FAILED;
      }
      *handled = TRUE;
      return ERR_OK;
    }
    return res;
}
#endif

void LINE_Init(void) {
  LINE_linePos = 0;
  lineState = LINE_STATE_INIT;
  vSemaphoreCreateBinary(LINE_StartStopCalibSem);
  if (LINE_StartStopCalibSem==NULL) { /* semaphore creation failed */
    for(;;){} /* error */
  }
  (void)xSemaphoreTake(LINE_StartStopCalibSem, 0); /* empty token */
  vQueueAddToRegistry(LINE_StartStopCalibSem, "LineStartStopCalibSem");
}

#endif /* PL_CONFIG_HAS_LINE */

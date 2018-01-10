/*
 * Quadrature.c
 *
 *  Created on: 14.12.2017
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "McuLib.h"
#include "Pin.h"
#include "Quadrature.h"
#if PL_CONFIG_HAS_SHELL
  #include "McuShell.h"
  #include "McuUtility.h"
#endif

#define Q4CLeft_SWAP_PINS  				0 /* 1: C1 and C2 are swapped */
#define Q4CLeft_SWAP_PINS_AT_RUNTIME  	0 /* 1: C1 and C2 are swapped at runtime, if SwapPins() method is available */
#define Q4CLeft_GET_C1_PIN()      		((HAL_GPIO_ReadPin(PIN_ENCL_A_PORT, PIN_ENCL_A_PIN)==GPIO_PIN_SET)?1:0)
#define Q4CLeft_GET_C2_PIN()      		((HAL_GPIO_ReadPin(PIN_ENCL_B_PORT, PIN_ENCL_B_PIN)==GPIO_PIN_SET)?1:0)
#if Q4CLeft_SWAP_PINS
  #define Q4CLeft_GET_C1_C2_PINS()               ((Q4CLeft_GET_C2_PIN()!=0?2:0)|(Q4CLeft_GET_C1_PIN()!=0?1:0))
  #define Q4CLeft_GET_C1_C2_PINS_SWAPPED()       ((Q4CLeft_GET_C1_PIN()!=0?2:0)|(Q4CLeft_GET_C2_PIN()!=0?1:0))
#else
  #define Q4CLeft_GET_C1_C2_PINS()               ((Q4CLeft_GET_C1_PIN()!=0?2:0)|(Q4CLeft_GET_C2_PIN()!=0?1:0))
  #define Q4CLeft_GET_C1_C2_PINS_SWAPPED()       ((Q4CLeft_GET_C2_PIN()!=0?2:0)|(Q4CLeft_GET_C1_PIN()!=0?1:0))
#endif

#define Q4CRight_SWAP_PINS  				0 /* 1: C1 and C2 are swapped */
#define Q4CRight_SWAP_PINS_AT_RUNTIME  		0 /* 1: C1 and C2 are swapped at runtime, if SwapPins() method is available */
#define Q4CRight_GET_C1_PIN()      			((HAL_GPIO_ReadPin(PIN_ENCR_A_PORT, PIN_ENCR_A_PIN)==GPIO_PIN_SET)?1:0)
#define Q4CRight_GET_C2_PIN()      			((HAL_GPIO_ReadPin(PIN_ENCR_B_PORT, PIN_ENCR_B_PIN)==GPIO_PIN_SET)?1:0)
#if Q4CRight_SWAP_PINS
  #define Q4CRight_GET_C1_C2_PINS()               ((Q4CRight_GET_C2_PIN()!=0?2:0)|(Q4CRight_GET_C1_PIN()!=0?1:0))
  #define Q4CRight_GET_C1_C2_PINS_SWAPPED()       ((Q4CRight_GET_C1_PIN()!=0?2:0)|(Q4CRight_GET_C2_PIN()!=0?1:0))
#else
  #define Q4CRight_GET_C1_C2_PINS()               ((Q4CRight_GET_C1_PIN()!=0?2:0)|(Q4CRight_GET_C2_PIN()!=0?1:0))
  #define Q4CRight_GET_C1_C2_PINS_SWAPPED()       ((Q4CRight_GET_C2_PIN()!=0?2:0)|(Q4CRight_GET_C1_PIN()!=0?1:0))
#endif

/* The decoder has 4 different states, together with the previous state the table has 16 entries.
   The value in the table (0,1,-1) indicates the steps taken since previous sample. */
#define QUAD_ERROR  3 /*!< Value to indicate an error in impulse detection. Has to be different from 0,1,-1 */

static const signed char Quad_Table[4][4] =
  {               /* prev   new    */
    {             /* c1 c2  c1 c2  */
     0,           /* 0  0   0  0  no change or missed a step? */
     1,           /* 0  0   0  1   */
     -1,          /* 0  0   1  0   */
     QUAD_ERROR   /* 0  0   1  1  error, lost impulse */
     },
    {             /* c1 c2  c1 c2  */
     -1,          /* 0  1   0  0   */
     0,           /* 0  1   0  1   no change or missed a step? */
     QUAD_ERROR,  /* 0  1   1  0   error, lost impulse */
     1            /* 0  1   1  1   */
     },
    {             /* c1 c2  c1 c2  */
     1,           /* 1  0   0  0   */
     QUAD_ERROR,  /* 1  0   0  1   error, lost impulse */
     0,           /* 1  0   1  0   no change or missed a step? */
     -1           /* 1  0   1  1   */
     },
    {             /* c1 c2  c1 c2  */
     QUAD_ERROR,  /* 1  1   0  0   error, lost impulse */
     -1,          /* 1  1   0  1   */
     1,           /* 1  1   1  0   */
     0            /* 1  1   1  1   no change or missed a step? */
     }
  };
static uint8_t Q4CLeft_last_quadrature_value; /*! Value of C1&C2 during last round. */
static uint8_t Q4CRight_last_quadrature_value; /*! Value of C1&C2 during last round. */

static QUAD_QuadCntrType Q4CLeft_currPos, Q4CRight_currPos;
static uint32_t Q4CLeft_nofErrors, Q4CRight_nofErrors;

uint32_t QUAD_NofLeftErrors(void) {
	return Q4CLeft_nofErrors;
}
uint32_t QUAD_NofRightErrors(void){
	return Q4CRight_nofErrors;
}

QUAD_QuadCntrType QUAD_GetLeftPos(void) {
	return Q4CLeft_currPos;
}

QUAD_QuadCntrType QUAD_GetRightPos(void) {
	return Q4CRight_currPos;
}

uint8_t Q4CLeft_GetVal(void) {
#if Q4CLeft_SWAP_PINS_AT_RUNTIME
  if (Q4CLeft_swappedPins) {
    return Q4CLeft_GET_C1_C2_PINS();
  } else {
    return Q4CLeft_GET_C1_C2_PINS_SWAPPED();
  }
#else
  return Q4CLeft_GET_C1_C2_PINS();
#endif
}

uint8_t Q4CRight_GetVal(void) {
#if Q4CRight_SWAP_PINS_AT_RUNTIME
  if (Q4CRight_swappedPins) {
    return Q4CRight_GET_C1_C2_PINS();
  } else {
    return Q4CRight_GET_C1_C2_PINS_SWAPPED();
  }
#else
  return Q4CRight_GET_C1_C2_PINS();
#endif
}

static void QUAD_SampleLeft(void) {
  signed char new_step;
  uint8_t c12; /* value of the two sensor input */

  c12 = Q4CLeft_GetVal();
  new_step = Quad_Table[Q4CLeft_last_quadrature_value][c12];
  Q4CLeft_last_quadrature_value = c12;
  if (new_step == QUAD_ERROR) {
	Q4CLeft_nofErrors++;
  } else if (new_step != 0) {
	Q4CLeft_currPos += new_step;
  }
}

static void QUAD_SampleRight(void) {
  signed char new_step;
  uint8_t c12; /* value of the two sensor input */

  c12 = Q4CRight_GetVal();
  new_step = Quad_Table[Q4CRight_last_quadrature_value][c12];
  Q4CRight_last_quadrature_value = c12;
  if (new_step == QUAD_ERROR) {
	Q4CRight_nofErrors++;
  } else if (new_step != 0) {
	  Q4CRight_currPos += new_step;
  }
}

void QUAD_Sample(void) {
	QUAD_SampleLeft();
	QUAD_SampleRight();
}

#if PL_CONFIG_HAS_SHELL
uint8_t QUAD_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io)
{
  uint8_t res=ERR_OK;

  if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "Quad help")==0) {
    McuShell_SendHelpStr((const unsigned char*)"quad", (const unsigned char*)"Quadrature sensor command group\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  reset", (const unsigned char*)"Reset the current position counter\r\n", io->stdOut);
    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "quad status")==0) {
    McuShell_SendStr((const unsigned char*)"quad:\r\n", io->stdOut);
    McuShell_SendStatusStr((const unsigned char*)"  pos", (const unsigned char*)"", io->stdOut);
  #if QUAD_CNTR_BITS==16
    McuShell_SendNum16s((int16_t)Q4CLeft_currPos, io->stdOut);
    McuShell_SendStr((const unsigned char*)", ", io->stdOut);
    McuShell_SendNum16s((int16_t)Q4CRight_currPos, io->stdOut);
  #elif QUAD_CNTR_BITS==32
    McuShell_SendNum32s((int32_t)Q4CLeft_currPos, io->stdOut);
    McuShell_SendStr((const unsigned char*)", ", io->stdOut);
    McuShell_SendNum32s((int32_t)Q4CRight_currPos, io->stdOut);
  #else
    #error "unknown counter size!"
  #endif
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);
    McuShell_SendStatusStr((const unsigned char*)"  errors", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum16u(Q4CLeft_nofErrors, io->stdOut);
    McuShell_SendStr((const unsigned char*)", ", io->stdOut);
    McuShell_SendNum16u(Q4CRight_nofErrors, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);
    McuShell_SendStatusStr((const unsigned char*)"  C1 C2", (const unsigned char*)"", io->stdOut);
    if (Q4CLeft_GET_C1_PIN()!=0) {
      McuShell_SendStr((const unsigned char*)"[1,", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"[0,", io->stdOut);
    }
    if (Q4CLeft_GET_C2_PIN()!=0) {
      McuShell_SendStr((const unsigned char*)"1] ", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0] ", io->stdOut);
    }
    if (Q4CRight_GET_C1_PIN()!=0) {
      McuShell_SendStr((const unsigned char*)"[1,", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"[0,", io->stdOut);
    }
    if (Q4CRight_GET_C2_PIN()!=0) {
      McuShell_SendStr((const unsigned char*)"1]\r\n", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0]\r\n", io->stdOut);
    }
    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, "quad reset")==0) {
	QUAD_Reset();
    *handled = TRUE;
  }
  return res;
}
#endif /* PL_CONFIG_HAS_SHELL */

void QUAD_SetLeftPos(QUAD_QuadCntrType pos) {
	Q4CLeft_currPos = pos;
}

void QUAD_SetRightPos(QUAD_QuadCntrType pos) {
	Q4CRight_currPos = pos;
}

void QUAD_Reset(void) {
	Q4CLeft_currPos = 0;
	Q4CLeft_nofErrors = 0;
	Q4CRight_currPos = 0;
	Q4CRight_nofErrors = 0;
}

void QUAD_Init(void) {
	QUAD_Reset();
}

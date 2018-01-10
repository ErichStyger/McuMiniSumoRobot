/**
 * \file
 * \brief Implementation of a driver for the Pins on the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements functions for the Pins (switches, push buttons and tilt switch).
 */

#include "Platform.h"
#include "Pin.h"
#include <stdbool.h>
#if PL_CONFIG_BOARD==PL_CONFIG_BOARD_ID_STM32_NUCLEO
  #include "stm32f3xx_hal.h"
#endif

#define SW3_Get()   (HAL_GPIO_ReadPin(PIN_SW3_PORT, PIN_SW3_PIN)==GPIO_PIN_SET)
	/*!< returns the status of the SW3 switch */
#define ENCL_A_Get()   (HAL_GPIO_ReadPin(PIN_ENCL_A_PORT, PIN_ENCL_A_PIN)==GPIO_PIN_SET)
#define ENCL_B_Get()   (HAL_GPIO_ReadPin(PIN_ENCL_B_PORT, PIN_ENCL_B_PIN)==GPIO_PIN_SET)
#define ENCR_A_Get()   (HAL_GPIO_ReadPin(PIN_ENCR_A_PORT, PIN_ENCR_A_PIN)==GPIO_PIN_SET)
#define ENCR_B_Get()   (HAL_GPIO_ReadPin(PIN_ENCR_B_PORT, PIN_ENCR_B_PIN)==GPIO_PIN_SET)

#define PROX_L_Get()   (HAL_GPIO_ReadPin(PIN_PROX_L_PORT, PIN_PROX_L_PIN)==GPIO_PIN_SET)
#define PROX_M_Get()   (HAL_GPIO_ReadPin(PIN_PROX_M_PORT, PIN_PROX_M_PIN)==GPIO_PIN_SET)
#define PROX_R_Get()   (HAL_GPIO_ReadPin(PIN_PROX_R_PORT, PIN_PROX_R_PIN)==GPIO_PIN_SET)

#define PROX_IR_SELECT_HIGH() 	HAL_GPIO_WritePin(PIN_PROX_IR_SELECT_PORT, PIN_PROX_IR_SELECT_PIN, GPIO_PIN_SET)
#define PROX_IR_SELECT_LOW()  	HAL_GPIO_WritePin(PIN_PROX_IR_SELECT_PORT, PIN_PROX_IR_SELECT_PIN, GPIO_PIN_RESET)
#define PROX_IR_SELECT_TOGGLE()  HAL_GPIO_TogglePin(PIN_PROX_IR_SELECT_PORT, PIN_PROX_IR_SELECT_PIN)

#define EDGE_L_Get()    (HAL_GPIO_ReadPin(PIN_EDGE_L_PORT, PIN_EDGE_L_PIN)==GPIO_PIN_SET)
#define EDGE_ML_Get()   (HAL_GPIO_ReadPin(PIN_EDGE_ML_PORT, PIN_EDGE_ML_PIN)==GPIO_PIN_SET)
#define EDGE_MR_Get()   (HAL_GPIO_ReadPin(PIN_EDGE_MR_PORT, PIN_EDGE_MR_PIN)==GPIO_PIN_SET)
#define EDGE_R_Get()    (HAL_GPIO_ReadPin(PIN_EDGE_R_PORT, PIN_EDGE_R_PIN)==GPIO_PIN_SET)

#define MOT_DIR_L_HIGH() 	HAL_GPIO_WritePin(PIN_DIRL_PORT, PIN_DIRL_PIN, GPIO_PIN_SET)
#define MOT_DIR_L_LOW()  	HAL_GPIO_WritePin(PIN_DIRL_PORT, PIN_DIRL_PIN, GPIO_PIN_RESET)
#define MOT_DIR_L_TOGGLE()  HAL_GPIO_TogglePin(PIN_DIRL_PORT, PIN_DIRL_PIN)

#define MOT_DIR_R_HIGH() 	HAL_GPIO_WritePin(PIN_DIRR_PORT, PIN_DIRR_PIN, GPIO_PIN_SET)
#define MOT_DIR_R_LOW()  	HAL_GPIO_WritePin(PIN_DIRR_PORT, PIN_DIRR_PIN, GPIO_PIN_RESET)
#define MOT_DIR_R_TOGGLE()  HAL_GPIO_TogglePin(PIN_DIRR_PORT, PIN_DIRR_PIN)

#if PL_CONFIG_HAS_I2C
#define I2C_SDA_Get()       (HAL_GPIO_ReadPin(PIN_I2C_SDA_PORT, PIN_I2C_SDA_PIN)==GPIO_PIN_SET)
#define I2C_SDA_HIGH() 		HAL_GPIO_WritePin(PIN_I2C_SDA_PORT, PIN_I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_LOW()  		HAL_GPIO_WritePin(PIN_I2C_SDA_PORT, PIN_I2C_SDA_PIN, GPIO_PIN_RESET)
#define I2C_SDA_TOGGLE()  	HAL_GPIO_TogglePin(PIN_I2C_SDA_PORT, PIN_I2C_SDA_PIN)

#define I2C_SCL_Get()       (HAL_GPIO_ReadPin(PIN_I2C_SCL_PORT, PIN_I2C_SCL_PIN)==GPIO_PIN_SET)
#define I2C_SCL_HIGH() 		HAL_GPIO_WritePin(PIN_I2C_SCL_PORT, PIN_I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_LOW()  		HAL_GPIO_WritePin(PIN_I2C_SCL_PORT, PIN_I2C_SCL_PIN, GPIO_PIN_RESET)
#define I2C_SCL_TOGGLE()  	HAL_GPIO_TogglePin(PIN_I2C_SCL_PORT, PIN_I2C_SCL_PIN)
#endif

void PIN_SetDirection(Pin_PinId pin, bool isOutput) {
  GPIO_InitTypeDef GPIO_InitStruct;
  uint32_t pinNr;
  GPIO_TypeDef *port;

  switch(pin) {
#if PL_CONFIG_HAS_SW_I2C
	  case PIN_I2C_SDA: port = PIN_I2C_SDA_PORT; pinNr = PIN_I2C_SDA_PIN; break;
	  case PIN_I2C_SCL: port = PIN_I2C_SCL_PORT; pinNr = PIN_I2C_SCL_PIN; break;
#endif
	  default:
		  for(;;) {} /* error! */
		  break;
  }
  GPIO_InitStruct.Alternate = 0; /* init to default value */
  if (isOutput) {
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* configure as push/pull output pin */
	  GPIO_InitStruct.Pull = GPIO_NOPULL; /* no pull-up needed */
  } else { /* input */
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; /* configure as input pin */
	  GPIO_InitStruct.Pull = GPIO_NOPULL; /* we have external pull-ups on the pins */
  }
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = pinNr;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

bool PIN_IsPinHigh(Pin_PinId pin) {
  switch(pin) {
    case PIN_SW3: return SW3_Get();
    case PIN_ENCL_A: return ENCL_A_Get();
    case PIN_ENCL_B: return ENCL_B_Get();
    case PIN_ENCR_A: return ENCR_A_Get();
    case PIN_ENCR_B: return ENCR_B_Get();
    case PIN_PROX_L: return PROX_L_Get();
    case PIN_PROX_M: return PROX_M_Get();
    case PIN_PROX_R: return PROX_R_Get();
    case PIN_EDGE_L: return EDGE_L_Get();
    case PIN_EDGE_ML: return EDGE_ML_Get();
    case PIN_EDGE_MR: return EDGE_MR_Get();
    case PIN_EDGE_R: return EDGE_R_Get();
#if PL_CONFIG_HAS_I2C
    case PIN_I2C_SCL: return I2C_SCL_Get();
    case PIN_I2C_SDA: return I2C_SDA_Get();
#endif
    default: break;
  }
  return false;
}

bool PIN_IsPinLow(Pin_PinId pin) {
	return !PIN_IsPinHigh(pin);
}

void PIN_SetHigh(Pin_PinId pin) {
  switch(pin) {
	case PIN_PROX_IR_SELECT: PROX_IR_SELECT_HIGH(); break;
	case PIN_DIR_L: MOT_DIR_L_HIGH(); break;
	case PIN_DIR_R: MOT_DIR_R_HIGH(); break;
#if PL_CONFIG_HAS_I2C
	case PIN_I2C_SCL: I2C_SCL_HIGH(); break;
	case PIN_I2C_SDA: I2C_SDA_HIGH(); break;
#endif
	default: break;
  }
}

void PIN_SetLow(Pin_PinId pin) {
  switch(pin) {
	case PIN_PROX_IR_SELECT: PROX_IR_SELECT_LOW(); break;
	case PIN_DIR_L: MOT_DIR_L_LOW(); break;
	case PIN_DIR_R: MOT_DIR_R_LOW(); break;
#if PL_CONFIG_HAS_I2C
	case PIN_I2C_SCL: I2C_SCL_LOW(); break;
	case PIN_I2C_SDA: I2C_SDA_LOW(); break;
#endif
	default: break;
  }
}

void PIN_Toggle(Pin_PinId pin) {
  switch(pin) {
	case PIN_PROX_IR_SELECT: PROX_IR_SELECT_TOGGLE(); break;
	case PIN_DIR_L: MOT_DIR_L_TOGGLE(); break;
	case PIN_DIR_R: MOT_DIR_R_TOGGLE(); break;
#if PL_CONFIG_HAS_I2C
	case PIN_I2C_SCL: I2C_SCL_TOGGLE(); break;
	case PIN_I2C_SDA: I2C_SDA_TOGGLE(); break;
#endif
	default: break;
  }
}

void PIN_Init(void) {
  /* nothing required */
}

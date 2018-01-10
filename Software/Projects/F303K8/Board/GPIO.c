/**
 * \file
 * \brief Implementation of the GPIO on the PRG_G board.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the GPIO (General Purpose Input/Output) driver.
 */

#include "Platform.h"
#include "GPIO.h"
#include "stm32f3xx_hal.h"
#include "Pin.h"

/* Sumo:
 * PA0: PWMA
 * PA1: PWMB
 * PA3: DIRA			GPIO, output
 * PA4: DIRB			GPIO, output
 * PA5: ProxIRDetect
 * PA6: ENCRA			GPIO, input, pull-up
 * PA7: ENCRB			GPIO, input, pull-up
 * PA8: EdgeR
 * PA9: UART1_RX
 * PA10: UART1_TX
 * PA11: EdgeMR
 * PA12: ProxR

 * PB0: ENCLA			GPIO, input, pull-up
 * PB1: ENCLB			GPIO, input, pull-up
 * PB3: SW3				GPIO, input
 * PB4: EdgeL
 * PB5: EdgeML
 * PB6: I2C1_SCL
 * PB7: I2C1_SDA

 * PF0: ProxM
 * PF1: ProxL
 */

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE(); /* enable clock for GPIO A */
  __HAL_RCC_GPIOB_CLK_ENABLE(); /* enable clock for GPIO B */
  __HAL_RCC_GPIOF_CLK_ENABLE(); /* enable clock for GPIO F */

  GPIO_InitStruct.Alternate = 0; /* init to default value */

  /* ******************* Configure GPIO Input pins *****************************/
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; /* configure as input pin */
  GPIO_InitStruct.Pull = GPIO_NOPULL; /* we have external pull-ups on the pins */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  /* ************** Proximity Sensors ******************* */
  GPIO_InitStruct.Pin = PIN_PROX_L_PIN; HAL_GPIO_Init(PIN_PROX_L_PORT, &GPIO_InitStruct); /* PTF1 */
  GPIO_InitStruct.Pin = PIN_PROX_M_PIN; HAL_GPIO_Init(PIN_PROX_M_PORT, &GPIO_InitStruct); /* PTF0 */
  GPIO_InitStruct.Pin = PIN_PROX_R_PIN; HAL_GPIO_Init(PIN_PROX_R_PORT, &GPIO_InitStruct); /* PTA12 */

  /* ************** User Switch ******************* */
  GPIO_InitStruct.Pin = PIN_SW3_PIN; HAL_GPIO_Init(PIN_SW3_PORT, &GPIO_InitStruct); /* PTB3: SW3 */

  /* Encoder pins with pull-ups */
  GPIO_InitStruct.Pull = GPIO_PULLUP; /* pull-ups are DNP on board */
  GPIO_InitStruct.Pin = PIN_ENCL_A_PIN; HAL_GPIO_Init(PIN_ENCL_A_PORT, &GPIO_InitStruct); /* PTB0: ENCLA */
  GPIO_InitStruct.Pin = PIN_ENCL_B_PIN; HAL_GPIO_Init(PIN_ENCL_B_PORT, &GPIO_InitStruct); /* PTB1: ENCLB */
  GPIO_InitStruct.Pin = PIN_ENCR_A_PIN; HAL_GPIO_Init(PIN_ENCR_A_PORT, &GPIO_InitStruct); /* PTA6: ENCRA */
  GPIO_InitStruct.Pin = PIN_ENCR_B_PIN; HAL_GPIO_Init(PIN_ENCR_A_PORT, &GPIO_InitStruct); /* PTA7: ENCRB */

  /* ******************* Configure GPIO Output pins *****************************/
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* configure as push/pull output pin */
  GPIO_InitStruct.Pull = GPIO_NOPULL; /* no pull-up needed */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  /* write default output level and configure pins */
  /* PA3: DIRA			GPIO, output */
  HAL_GPIO_WritePin(PIN_DIRL_PORT, PIN_DIRL_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = PIN_DIRL_PIN; HAL_GPIO_Init(PIN_DIRL_PORT, &GPIO_InitStruct);
  /* PA4: DIRB			GPIO, output */
  HAL_GPIO_WritePin(PIN_DIRR_PORT, PIN_DIRR_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = PIN_DIRR_PIN; HAL_GPIO_Init(PIN_DIRR_PORT, &GPIO_InitStruct);

  /* proximity IR selection */
  HAL_GPIO_WritePin(PIN_PROX_IR_SELECT_PORT, PIN_PROX_IR_SELECT_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = PIN_PROX_IR_SELECT_PIN; HAL_GPIO_Init(PIN_PROX_IR_SELECT_PORT, &GPIO_InitStruct);

#if PL_CONFIG_HAS_SW_I2C
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; /* configure as input pin */
  GPIO_InitStruct.Pull = GPIO_NOPULL; /* we have external pull-ups on the pins */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  /* SCL */
  GPIO_InitStruct.Pin = PIN_I2C_SCL_PIN;
  HAL_GPIO_Init(PIN_I2C_SCL_PORT, &GPIO_InitStruct);
  /* SDA */
  GPIO_InitStruct.Pin = PIN_I2C_SDA_PIN;
  HAL_GPIO_Init(PIN_I2C_SDA_PORT, &GPIO_InitStruct);
#endif
}

void GPIO_Init(void) {
  MX_GPIO_Init();
}

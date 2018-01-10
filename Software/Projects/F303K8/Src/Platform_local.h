/*
 * Platform_local.h
 *
 *  Created on: 30.11.2017
 *      Author: Erich Styger
 */

#ifndef SRC_PLATFORM_LOCAL_H_
#define SRC_PLATFORM_LOCAL_H_

#define PL_CONFIG_IS_DAC_ROBOT      (1)

#define PL_CONFIG_HAS_TIMER         (1)
#define PL_CONFIG_HAS_SHELL 		    (1)
#define PL_CONFIG_HAS_EVENTS        (1)
#define PL_CONFIG_HAS_MOTOR         (1)

#define PL_CONFIG_HAS_KEYS          (1)
#define PL_CONFIG_NOF_KEYS          (1)
#define PL_CONFIG_HAS_KBI           (0) /* NYI */
#define PL_CONFIG_HAS_TRIGGER       (1)
#define PL_CONFIG_HAS_DEBOUNCE      (1 && PL_CONFIG_HAS_TRIGGER)

#define PL_CONFIG_HAS_PROXIMITY     (1)
#define PL_CONFIG_HAS_REFLECTANCE 	(1)
#define PL_CONFIG_HAS_LINE          (1 && PL_CONFIG_HAS_REFLECTANCE)

#define PL_CONFIG_HAS_QUADRATURE    (1 && PL_CONFIG_HAS_MOTOR)
#define PL_CONFIG_HIGH_RES_ENCODER  (1 && PL_CONFIG_HAS_QUADRATURE)
#define PL_CONFIG_HAS_MOTOR_TACHO   (1 && PL_CONFIG_HAS_QUADRATURE)

#define PL_CONFIG_HAS_PID           (1 && PL_CONFIG_HAS_MOTOR)
#define PL_CONFIG_HAS_SPEED_PID     (1 && PL_CONFIG_HAS_PID)
#define PL_CONFIG_HAS_POS_PID       (1 && PL_CONFIG_HAS_PID)
#define PL_CONFIG_HAS_LINE_PID      (1 && PL_CONFIG_HAS_PID)
#define PL_CONFIG_GO_DEADEND_BW     (0) /* NYI */

#define PL_CONFIG_HAS_DRIVE         (1 && PL_CONFIG_HAS_QUADRATURE)
#define PL_CONFIG_HAS_TURN          (1 && PL_CONFIG_HAS_QUADRATURE)

#define PL_CONFIG_HAS_UART          (0) /* NYI */
#define PL_CONFIG_HAS_CONFIG_NVM    (0) /* NYI */

#define PL_CONFIG_HAS_I2C           (1)
#define PL_CONFIG_HAS_HW_I2C        (1 && PL_CONFIG_HAS_I2C) /* otherwise uses SW I2C */
#define PL_CONFIG_HAS_SW_I2C        (!PL_CONFIG_HAS_HW_I2C && PL_CONFIG_HAS_I2C) /* otherwise uses SW I2C */

#define PL_CONFIG_HAS_LCD           (1 && PL_CONFIG_HAS_I2C)
#define PL_CONFIG_HAS_LCD_MENU      (1 && PL_CONFIG_HAS_LCD && PL_CONFIG_HAS_DEBOUNCE)

#define PL_CONFIG_HAS_LINE_FOLLOW   (0 && PL_CONFIG_HAS_MOTOR && PL_CONFIG_HAS_LINE && PL_CONFIG_HAS_LINE_PID)
#define PL_CONFIG_HAS_LINE_MAZE     (0 && PL_CONFIG_HAS_LINE_FOLLOW)
#define PL_CONFIG_HAS_SUMO          (1 && PL_CONFIG_HAS_DRIVE && PL_CONFIG_HAS_TURN)

#define PL_APP_LINE_FOLLOWING 0
#define PL_APP_LINE_MAZE      0
#define PL_DO_MINT            0
#define PL_IS_ZUMO_ROBOT      0
#define PL_IS_MOTOR_1_100     0
#define PL_IS_ROUND_ROBOT     0
#define PL_IS_TRACK_ROBOT     0
#define PL_IS_INTRO_ZUMO_ROBOT  0
#define PL_IS_INTRO_ZUMO_K22    0
#define PL_HAS_LIPO       		0
#define PL_IS_INTRO_ZUMO_ROBOT2 0
#define PL_IS_INTRO_ZUMO_K22    0
#define PL_SLOWER_SPEED         0

/* enable one of the below */
#define PL_CONGIG_DO_SUMO      (1 && PL_CONFIG_HAS_SUMO)
#define PL_CONFIG_DO_TEST_IR   (0 && PL_CONFIG_HAS_PROXIMITY && PL_CONFIG_HAS_TURN)
#define PL_CONFIG_DO_TEST_PUSH (0 && PL_CONFIG_HAS_MOTOR)

#if (PL_CONGIG_DO_SUMO+PL_CONFIG_DO_TEST_IR+PL_CONFIG_DO_TEST_PUSH)>1
  #error "Only one can be active!"
#endif

#endif /* SRC_PLATFORM_LOCAL_H_ */

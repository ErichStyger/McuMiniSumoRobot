/**
 * \file
 * \brief Configuration header file for Kinetis Tools
 *
 * This header file is used to configure settings of the Kinetis Tools module.
 */

#ifndef __McuArmTools_CONFIG_H
#define __McuArmTools_CONFIG_H

#if !defined(McuArmTools_CONFIG_PARSE_COMMAND_ENABLED)
  #define McuArmTools_CONFIG_PARSE_COMMAND_ENABLED  (1)
    /*!< 1: shell support enabled, 0: otherwise */
#endif

#if McuLib_CONFIG_NXP_SDK_2_0_USED
  /* will include system header file in the implementation file */
#elif McuLib_CONFIG_SDK_VERSION_USED==McuLib_CONFIG_SDK_KINETIS_1_3
  /* will include system header file in the implementation file */
#elif McuLib_CONFIG_CPU_IS_ARM_CORTEX_M
  /* include here the low level CMSIS header files, e.g. with */
  #if McuLib_CONFIG_CPU_IS_STM32
    #include "stm32f3xx_hal.h" /* header file for STM32F303K8 */
  #endif
#endif

#endif /* __McuArmTools_CONFIG_H */

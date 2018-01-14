/**
 * \file
 * \brief FreeRTOS application hooks.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the optional hooks for FreeRTOS.
 */

#include "Platform.h"

#if PL_CONFIG_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#if PL_CONFIG_HAS_MOTOR_TACHO
  #include "Tacho.h"
#endif
#if PL_CONFIG_HAS_TRIGGER
  #include "Trigger.h"
#endif

void McuRTOS_vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
  /* This will get called if a stack overflow is detected during the context
     switch.  Set configCHECK_FOR_STACK_OVERFLOWS to 2 to also check for stack
     problems within nested interrupts, but only do this for debug purposes as
     it will increase the context switch time. */
  (void)pxTask;
  (void)pcTaskName;
  taskDISABLE_INTERRUPTS();
  /* Write your code here ... */
  for(;;) {}
}

void McuRTOS_vApplicationTickHook(void)
{
  /* Called for every RTOS tick (configTICK_RATE_HZ). */
#if PL_CONFIG_HAS_MOTOR_TACHO
  TACHO_Sample();
#endif
#if PL_CONFIG_HAS_TRIGGER
  TRG_AddTick();
#endif
}

void McuRTOS_vApplicationIdleHook(void)
{
  /* Called whenever the RTOS is idle (from the IDLE task).
     Here would be a good place to put the CPU into low power mode. */
  /* Write your code here ... */
}

void McuRTOS_vApplicationMallocFailedHook(void)
{
  /* Called if a call to pvPortMalloc() fails because there is insufficient
     free memory available in the FreeRTOS heap.  pvPortMalloc() is called
     internally by FreeRTOS API functions that create tasks, queues, software
     timers, and semaphores.  The size of the FreeRTOS heap is set by the
     configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  taskDISABLE_INTERRUPTS();
  /* Write your code here ... */
  for(;;) {}
}

#if configUSE_TICKLESS_IDLE == 1
void McuRTOS_vOnPreSleepProcessing(TickType_t expectedIdleTicks) {
}
#endif

#endif /* PL_CONFIG_USE_FREERTOS */

/**
 * \file
 * \brief Configuration header file for GenericI2C
 *
 * This header file is used to configure settings of the Generic I2C module.
 */

#ifndef __McuGenericI2C_CONFIG_H
#define __McuGenericI2C_CONFIG_H

#if !defined(McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT)
  #define McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT    (0)
    /*!< 1: generate user events for requesting bus; 0: no user events */
  #define McuGenericI2C_CONFIG_ON_REQUEST_BUS_EVENT   McuGenericI2C_OnRequestBus
  void McuGenericI2C_CONFIG_ON_REQUEST_BUS_EVENT(void); /* prototype */
#endif

#if !defined(McuGenericI2C_CONFIG_USE_ON_RELEASE_BUS_EVENT)
  #define McuGenericI2C_CONFIG_USE_ON_RELEASE_BUS_EVENT    (0)
    /*!< 1: generate user events for releasing bus; 0: no user events */
  #define McuGenericI2C_CONFIG_ON_RELEASE_BUS_EVENT   McuGenericI2C_OnReleaseBus
  void McuGenericI2C_CONFIG_ON_RELEASE_BUS_EVENT(void); /* prototype */
#endif

#if !defined(McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT)
  #define McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT    (0)
    /*!< 1: generate user events for errors; 0: no error events */
  #define McuGenericI2C_CONFIG_ON_ERROR_EVENT   McuGenericI2C_OnError
  void McuGenericI2C_CONFIG_ON_ERROR_EVENT(void); /* prototype */
#endif

#if !defined(McuGenericI2C_CONFIG_USE_MUTEX)
  #define McuGenericI2C_CONFIG_USE_MUTEX             (0)
    /*!< 1: Use a mutex to protect access to the bus; 0: no mutex used */
#endif

#if !defined(McuGenericI2C_CONFIG_WRITE_BUFFER_SIZE)
  #define McuGenericI2C_CONFIG_WRITE_BUFFER_SIZE             (32)
    /*!< Size of the write buffer size which defines the maximum block size which can be sent */
#endif

/* configuration of function names used for low level I2C functions */
#include "Platform.h"

#if PL_CONFIG_HAS_SW_I2C
#include "McuGenericSWI2C.h" /* interface of low level I2C driver */
#define McuGenericI2C_CONFIG_RECV_BLOCK                        McuGenericSWI2C_RecvBlock
#define McuGenericI2C_CONFIG_SEND_BLOCK                        McuGenericSWI2C_SendBlock
#define McuGenericI2C_CONFIG_SEND_STOP                         McuGenericSWI2C_SendStop
#define McuGenericI2C_CONFIG_SELECT_SLAVE                      McuGenericSWI2C_SelectSlave
#define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM                 McuGenericSWI2C_RecvBlockCustom
#define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM_AVAILABLE       (defined(McuGenericSWI2C_RECVBLOCKCUSTOM_AVAILABLE) && (McuGenericSWI2C_RECVBLOCKCUSTOM_AVAILABLE==1))
#else
#include "McuSTM32HALI2C.h" /* interface of low level I2C driver */
#define McuGenericI2C_CONFIG_RECV_BLOCK                        McuSTM32HALI2C_RecvBlock
#define McuGenericI2C_CONFIG_SEND_BLOCK                        McuSTM32HALI2C_SendBlock
#define McuGenericI2C_CONFIG_SEND_STOP                         McuSTM32HALI2C_SendStop
#define McuGenericI2C_CONFIG_SELECT_SLAVE                      McuSTM32HALI2C_SelectSlave
#define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM                 McuSTM32HALI2C_RecvBlockCustom
#define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM_AVAILABLE       (defined(McuSTM32HALI2C_RECVBLOCKCUSTOM_AVAILABLE) && (McuSTM32HALI2C_RECVBLOCKCUSTOM_AVAILABLE==1))
#endif
#endif /* __McuGenericI2C_CONFIG_H */

/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCI_SCFW_H
#define SCI_SCFW_H

/* Includes */

#include <stdint.h>

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

/*!
 * This type is used to declare a handle for an IPC communication
 * channel. Its meaning is specific to the IPC implementation.
 */
typedef uint64_t sc_ipc_t;

/*!
 * This type is used to declare an ID for an IPC communication
 * channel. For the reference IPC implementation, this ID
 * selects the base address of the MU used for IPC.
 */
typedef uint64_t sc_ipc_id_t;


#endif /* SCI_SCFW_H */

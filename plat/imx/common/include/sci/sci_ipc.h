/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file for the IPC implementation.
 */

#ifndef SCI_IPC_H
#define SCI_IPC_H

/* Includes */

#include <sci/sci_types.h>

/* Defines */

/* Types */

/* Functions */

/*!
 * This function opens an IPC channel.
 *
 * @param[out]    ipc         return pointer for ipc handle
 * @param[in]     id          id of channel to open
 *
 * @return Returns an error code (SC_ERR_NONE = success, SC_ERR_IPC
 *         otherwise).
 *
 * The \a id parameter is implementation specific. Could be an MU
 * address, pointer to a driver path, channel index, etc.
 */
sc_err_t sc_ipc_open(sc_ipc_t *ipc, sc_ipc_id_t id);

/*!
 * This function closes an IPC channel.
 *
 * @param[in]     ipc         id of channel to close
 */
void sc_ipc_close(sc_ipc_t ipc);

/*!
 * This function reads a message from an IPC channel.
 *
 * @param[in]     ipc         id of channel read from
 * @param[out]    data        pointer to message buffer to read
 *
 * This function will block if no message is available to be read.
 */
void sc_ipc_read(sc_ipc_t ipc, void *data);

/*!
 * This function writes a message to an IPC channel.
 *
 * @param[in]     ipc         id of channel to write to
 * @param[in]     data        pointer to message buffer to write
 *
 * This function will block if the outgoing buffer is full.
 */
void sc_ipc_write(sc_ipc_t ipc, void *data);

sc_ipc_t ipc_handle;

#endif /* SCI_IPC_H */

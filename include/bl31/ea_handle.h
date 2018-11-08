/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EA_HANDLE_H
#define EA_HANDLE_H

/* Constants indicating the reason for an External Abort */

/* External Abort received at SError vector */
#define ERROR_EA_ASYNC		0

/* Synchronous External Abort received at Synchronous exception vector */
#define ERROR_EA_SYNC		1

/* External Abort synchronized by ESB instruction */
#define ERROR_EA_ESB		2

/* RAS event signalled as peripheral interrupt */
#define ERROR_INTERRUPT		3

#endif /* EA_HANDLE_H */

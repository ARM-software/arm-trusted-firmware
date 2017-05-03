/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LIB_SM_SM_ERR_H
#define __LIB_SM_SM_ERR_H

/* Errors from the secure monitor */
#define SM_ERR_UNDEFINED_SMC		0xFFFFFFFF /* Unknown SMC (defined by ARM DEN 0028A(0.9.0) */
#define SM_ERR_INVALID_PARAMETERS	-2
#define SM_ERR_INTERRUPTED		-3	/* Got interrupted. Call back with restart SMC */
#define SM_ERR_UNEXPECTED_RESTART	-4	/* Got an restart SMC when we didn't expect it */
#define SM_ERR_BUSY			-5	/* Temporarily busy. Call back with original args */
#define SM_ERR_INTERLEAVED_SMC		-6	/* Got a trusted_service SMC when a restart SMC is required */
#define SM_ERR_INTERNAL_FAILURE		-7	/* Unknown error */
#define SM_ERR_NOT_SUPPORTED		-8
#define SM_ERR_NOT_ALLOWED		-9	/* SMC call not allowed */
#define SM_ERR_END_OF_INPUT		-10

#endif

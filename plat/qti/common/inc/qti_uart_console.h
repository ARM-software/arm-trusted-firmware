/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#ifndef __QTI_UART_CONSOLE_H__
#define __QTI_UART_CONSOLE_H__

#include <console.h>

#define QTI_CONSOLE_T_UART_BASE_OFF		(CONSOLE_T_DRVDATA)

#ifndef __ASSEMBLER__

typedef struct {
	console_t console;
	uintptr_t base;
} qti_console_uart_t;

int qti_console_uart_register(qti_console_uart_t * console,
			      uintptr_t uart_base_addr);

#endif /* __ASSEMBLER__ */

#endif /* __QTI_UART_CONSOLE_H__ */

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CBMEM_CONSOLE_H__
#define __CBMEM_CONSOLE_H__

#include <console.h>

#define CONSOLE_T_CBMC_BASE	CONSOLE_T_DRVDATA
#define CONSOLE_T_CBMC_SIZE	(CONSOLE_T_DRVDATA + REGSZ)

#ifndef __ASSEMBLER__

typedef struct {
	console_t console;
	uintptr_t base;
	uint32_t size;
} console_cbmc_t;

int console_cbmc_register(console_cbmc_t *console, uintptr_t base);

#endif /* __ASSEMBLER__ */

#endif /* __CBMEM_CONSOLE_H__ */

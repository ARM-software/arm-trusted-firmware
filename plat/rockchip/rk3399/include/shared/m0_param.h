/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __M0_PARAM_H__
#define __M0_PARAM_H__

#ifndef __LINKER__
enum {
	M0_FUNC_SUSPEND = 0,
	M0_FUNC_DRAM	= 1,
};
#endif /* __LINKER__ */

#define PARAM_ADDR		0xc0

#define PARAM_M0_FUNC		0x00
#define PARAM_DRAM_FREQ		0x04
#define PARAM_DPLL_CON0		0x08
#define PARAM_DPLL_CON1		0x0c
#define PARAM_DPLL_CON2		0x10
#define PARAM_DPLL_CON3		0x14
#define PARAM_DPLL_CON4		0x18
#define PARAM_DPLL_CON5		0x1c
#define PARAM_FREQ_SELECT	0x20
#define PARAM_M0_DONE		0x24
#define PARAM_M0_SIZE		0x28
#define M0_DONE_FLAG		0xf59ec39a

#endif /*__M0_PARAM_H__*/

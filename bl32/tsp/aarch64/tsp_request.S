/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <bl32/tsp/tsp.h>

	.globl tsp_get_magic

/*
 * This function raises an SMC to retrieve arguments from secure
 * monitor/dispatcher, saves the returned arguments the array received in x0,
 * and then returns to the caller
 */
func tsp_get_magic
	/* Load arguments */
	ldr	w0, _tsp_fid_get_magic

	/* Raise SMC */
	smc	#0

	/* Return arguments in x1:x0 */
	ret
endfunc tsp_get_magic

	.align 2
_tsp_fid_get_magic:
	.word	TSP_GET_ARGS

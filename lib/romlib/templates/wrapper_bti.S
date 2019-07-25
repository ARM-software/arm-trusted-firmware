/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
	.globl	${function_name}
${function_name}:
	bti	jc
	ldr	x17, =jmptbl
	mov	x16, #${function_offset}
	ldr	x17, [x17]
	add	x16, x16, x17
	br	x16

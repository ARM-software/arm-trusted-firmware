/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* convoluted way to make sure that the define is pasted just the right way */
#define _INCBIN(file, sym) \
	__asm__( \
		".section .sram.incbin\n" \
		".global " #sym "\n" \
		".type " #sym ", %object\n" \
		".align 4\n" \
		#sym ":\n" \
		".incbin \"" #file "\"\n" \
		".size " #sym ", .-" #sym "\n" \
		".global " #sym "_end\n" \
		#sym "_end:\n" \
	)

#define INCBIN(file, sym) _INCBIN(file, sym)

INCBIN(RK3399M0FW, rk3399m0_bin);

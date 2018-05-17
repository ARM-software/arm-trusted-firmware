/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* convoluted way to make sure that the define is pasted just the right way */
#define _INCBIN(file, sym, sec) \
	__asm__( \
		".section " #sec "\n" \
		".global " #sym "\n" \
		".type " #sym ", %object\n" \
		".align 4\n" \
		#sym ":\n" \
		".incbin \"" #file "\"\n" \
		".size " #sym ", .-" #sym "\n" \
		".global " #sym "_end\n" \
		#sym "_end:\n" \
	)

#define INCBIN(file, sym, sec) _INCBIN(file, sym, sec)

INCBIN(RK3399M0FW, rk3399m0_bin, ".sram.incbin");
INCBIN(RK3399M0PMUFW, rk3399m0pmu_bin, ".pmusram.incbin");

/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "plat/arm/common/arm_def.h"

	.global arm_rotpk_header
	.global arm_rotpk_header_end
	.section .rodata.arm_rotpk_hash, "a"

arm_rotpk_header:
	.byte 0x30, 0x31, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48
	.byte 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
arm_rotpk_header_len:

#ifdef ARM_ROTPK_HASH
	.global arm_rotpk_hash_end
	.incbin ARM_ROTPK_HASH
arm_rotpk_hash_end:
#endif

.if ARM_ROTPK_HEADER_LEN != arm_rotpk_header_len - arm_rotpk_header
.error "Invalid ROTPK header length."
.endif

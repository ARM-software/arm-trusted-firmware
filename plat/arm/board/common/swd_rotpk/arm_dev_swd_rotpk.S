/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.global arm_swd_rotpk_hash
	.global arm_swd_rotpk_hash_end

	.section .rodata.arm_swd_rotpk_hash, "a"

arm_swd_rotpk_hash:
	/* DER header. */
	.byte 0x30, 0x31, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48
	.byte 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
	/* Key hash. */
	.incbin ARM_SWD_ROTPK_HASH
arm_swd_rotpk_hash_end:

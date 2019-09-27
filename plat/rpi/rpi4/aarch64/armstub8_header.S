/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * armstub8.bin header to let the GPU firmware recognise this code.
 * It will then write the load address of the kernel image and the DT
 * after the header magic in RAM, so we can read those addresses at runtime.
 */

.text
	b	armstub8_end

.global stub_magic
.global dtb_ptr32
.global kernel_entry32

.org 0xf0
armstub8:
stub_magic:
	.word 0x5afe570b
stub_version:
	.word 0
dtb_ptr32:
	.word 0x0
kernel_entry32:
	.word 0x0

/*
 * Technically an offset of 0x100 would suffice, but the follow-up code
 * (bl31_entrypoint.S at BL31_BASE) needs to be page aligned, so pad here
 * till the end of the first 4K page.
 */
.org 0x1000
armstub8_end:

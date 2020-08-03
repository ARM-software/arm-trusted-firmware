/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The Arm Ltd. FPGA images start execution at address 0x0, which is
 * mapped at an (emulated) ROM image. The payload uploader can write to
 * this memory, but write access by the CPU cores is prohibited.
 *
 * Provide a simple trampoline to start BL31 execution at the actual
 * load address. We put the DTB address in x0, so any code in DRAM could
 * make use of that information (not yet used in BL31 right now).
 */

#include <asm_macros.S>
#include <common/bl_common.ld.h>

.text
.global _start

_start:
	mov_imm	x1, BL31_BASE			/* beginning of DRAM */
	mov_imm	x0, FPGA_PRELOADED_DTB_BASE
	br	x1

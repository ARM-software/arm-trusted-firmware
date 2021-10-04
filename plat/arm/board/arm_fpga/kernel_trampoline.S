/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The traditional arm64 Linux kernel load address is 512KiB from the
 * beginning of DRAM, caused by this having been the default value of the
 * kernel's CONFIG_TEXT_OFFSET Kconfig value.
 * However kernel version 5.8 changed the default offset (into a 2MB page)
 * to 0, so TF-A's default assumption is no longer true. Fortunately the
 * kernel got more relaxed about this offset at the same time, so it
 * tolerates the wrong offset, but issues a warning:
 * [Firmware Bug]: Kernel image misaligned at boot, please fix your bootloader!
 *
 * We cannot easily change the load address offset in TF-A to be 2MiB, because
 * this would break older kernels - and they are not as forgiving in this
 * respect.
 *
 * But we can allow users to load the kernel at the right offset, and
 * offer this trampoline here to transition to this new load address.
 * Any older kernels, or newer kernels misloaded, will overwrite this code
 * here, so it does no harm in this case.
 */

#include <asm_macros.S>
#include <common/bl_common.ld.h>

.text
.global _tramp_start

_tramp_start:
	adr	x4, _tramp_start
	orr	x4, x4, #0x1fffff
	add	x4, x4, #1			/* align up to 2MB */
	br	x4

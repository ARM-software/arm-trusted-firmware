/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RCAR_PLAT_LD_S
#define RCAR_PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

MEMORY {
    SRAM      (rwx): ORIGIN = DEVICE_SRAM_BASE, LENGTH = DEVICE_SRAM_SIZE
    SRAM_DATA (rwx): ORIGIN = DEVICE_SRAM_DATA_BASE, LENGTH = DEVICE_SRAM_DATA_SIZE
    PRAM      (r)  : ORIGIN = BL31_LIMIT - (DEVICE_SRAM_SIZE + DEVICE_SRAM_DATA_SIZE),
                     LENGTH = DEVICE_SRAM_SIZE + DEVICE_SRAM_DATA_SIZE
}

SECTIONS
{
	. = BL31_BASE - 0x1000;
	.preram_trampoline . : {
		/*
		 * The SCP firmware on this platform uses hard-coded
		 * entrypoints, primary at 0x8c200000 and secondary at
		 * 0x8c200230. The primary entrypoint has no problems,
		 * but the secondary entrypoint is placed in the middle
		 * of bl31_entrypoint function. This trampoline works
		 * around the SCP limitation by placing a 4 kiB page
		 * with immediate jump instructions before the start
		 * of BL31 text, immediately before the BL31 primary
		 * entrypoint. The trampoline contains the following
		 * components:
		 * - Offset 0x000
		 *   -> Jump to bl31_entrypoint , which is at offset
		 *      0x1000 from the current location in code.
		 *      The jump offset calculation works as follows:
		 *      - (bl31_entrypoint - BL31_BASE)
		 *        This is the BL31 primary entrypoint offset
		 *        inside the BL31 binary. For bl31_entrypoint
		 *        symbol, this currently evaluates to zero.
		 *      - Add + 0x1000
		 *        This code executes from BL31_BASE - 0x1000,
		 *        the jump has to land inside the BL31 binary,
		 *        which is placed at offset +0x1000. Add this
		 *        offset to land inside BL31_BASE binary.
		 *      - Divide by 4
		 *        The aarch64 B branch instruction imm26 field
		 *        encodes jump offset in increments of 1, which
		 *        maps to jump target address increments of 4.
		 * - Offset 0x000..0x22c
		 *   -> Jump to offset -0x4 , i.e. execute previous
		 *      instruction. This is a safeguard against a
		 *      jump in the middle of the page. If this code
		 *      is triggered, it will jump all the way to
		 *      offset 0x0, and from there to offset 0x1000,
		 *      which is the BL31 primary entrypoint.
		 * - Offset 0x230..0x238
		 *   -> Secondary entry point trampoline. This code
		 *      jumps directly to "plat_secondary_reset" via
		 *      "plat_secondary_reset" encoded into "imm26"
		 *      parameter of a "b" instruction. This encoding
		 *      can never overflow, because the BL31_LIMIT is
		 *      256 kiB, and maximum PC-relative jump offset
		 *      is ( 256 kiB - 0x230 - 4 ) / 4 = 0xff73, which
		 *      always comfortably fits into 26-bit "imm26"
		 *      branch instruction argument.
		 *      The jump offset calculation works as follows:
		 *      - (plat_secondary_reset - BL31_BASE)
		 *        This is the BL31 secondary entrypoint offset
		 *        inside the BL31 binary. This evaluates to a
		 *        non-zero offset.
		 *      - Add + 0x1000 and subtract - 0x230
		 *        This code executes from BL31_BASE - 0x1000 + 0x230,
		 *        the jump has to land inside the BL31 binary,
		 *        which is placed at offset + 0x1000 - 0x230.
		 *        Calculate this offset and add and subtract
		 *        this offset to land inside BL31_BASE binary.
		 *      - Divide by 4
		 *        The aarch64 B branch instruction imm26 field
		 *        encodes jump offset in increments of 1, which
		 *        maps to jump target address increments of 4.
		 * - Offset 0x234..0xffc
		 *   -> Jump to offset +0x4 , i.e. execute the next
		 *      instruction. This is a safeguard against a
		 *      jump in the middle of the page. If this code
		 *      is triggered, it will jump all the way to
		 *      offset 0x1000, which is the BL31 primary
		 *      entrypoint.
		 * - Offset 0x1000 (not included in this section)
		 *   -> This is bl31_entrypoint .
		 */
		/* b 0x1000 */
		LONG(0x14000000 + ((bl31_entrypoint - BL31_BASE + 0x1000) / 4));
		/* b -0x4 */
		FILL(0xffffff17);
		. += 0x230 - 0x4;
		/* b plat_secondary_reset */
		LONG(0x14000000 + ((plat_secondary_reset - BL31_BASE + 0x1000 - 0x230) / 4));
		/* b +0x4 */
		FILL(0x01000014);
		. += 0xdcc;
	}

	/* SRAM_COPY is in PRAM (RO) */
	. = BL31_LIMIT - (DEVICE_SRAM_SIZE + DEVICE_SRAM_DATA_SIZE);
	__SRAM_COPY_START__ = .;

	.system_ram : {
		/* system ram start is in SRAM */
		__system_ram_start__ = .;
		*(.system_ram)
		__system_ram_end__ = .;
	} >SRAM AT>PRAM

	/* SRAM variable is in PRAM (RW) */
	. = BL31_LIMIT - DEVICE_SRAM_DATA_SIZE;

	.system_ram_data : {
		__system_ram_data_start__ = .;
		*(.system_ram_data)
		__system_ram_data_end__ = .;
	} >SRAM_DATA AT>PRAM

	ASSERT(__BL31_END__ <= (BL31_LIMIT - (DEVICE_SRAM_SIZE + DEVICE_SRAM_DATA_SIZE)),
	"BL31 image too large - writing on top of SRAM!")
}

#endif /* RCAR_PLAT_LD_S */

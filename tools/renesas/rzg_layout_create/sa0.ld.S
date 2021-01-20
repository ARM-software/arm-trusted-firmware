/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
	. = 0x00000000;
	.rodata : {
		KEEP(*(.sa0_bootrom))
		/* Map Type 3 for eMMC Boot */
                /* A-side IPL content cert "Start Address" */
		. = 0x000001D4;		/* H'00000080 + H'00000154 */
		KEEP(*(.sa0_bl2dst_addr3))
                /* A-side IPL content cert "Size" */
		. = 0x000002E4;		/* H'00000080 + H'00000264 */
		KEEP(*(.sa0_bl2dst_size3))
		/* Map Type 1 for HyperFlash/QSPI Flash Boot */
		/* A-side IPL content cert "Start Address" */
		. = 0x00000D54;		/* H'00000C00 + H'00000154 */
		KEEP(*(.sa0_bl2dst_addr1))
		/* A-side IPL content cert "Size" */
		. = 0x00000E64;		/* H'00000C00 + H'00000264 */
		KEEP(*(.sa0_bl2dst_size1))
	}

}

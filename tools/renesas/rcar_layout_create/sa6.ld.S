/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
	. = 0x00000000;
	.rodata : {
		KEEP(*(.sa6_image_num))
		. = 0x00000008;
		KEEP(*(.sa6_bl31src_addr))
		. = 0x00000010;
		KEEP(*(.sa6_bl31partition))
		. = 0x00000018;
		KEEP(*(.sa6_bl32src_addr))
		. = 0x00000020;
		KEEP(*(.sa6_bl32partition))
		. = 0x00000028;
		KEEP(*(.sa6_bl33src_addr))
		. = 0x00000030;
		KEEP(*(.sa6_bl33partition))
		. = 0x00000038;
		KEEP(*(.sa6_bl332src_addr))
		. = 0x00000040;
		KEEP(*(.sa6_bl332partition))
		. = 0x00000048;
		KEEP(*(.sa6_bl333src_addr))
		. = 0x00000050;
		KEEP(*(.sa6_bl333partition))
		. = 0x00000058;
		KEEP(*(.sa6_bl334src_addr))
		. = 0x00000060;
		KEEP(*(.sa6_bl334partition))
		. = 0x00000068;
		KEEP(*(.sa6_bl335src_addr))
		. = 0x00000070;
		KEEP(*(.sa6_bl335partition))
		. = 0x00000078;
		KEEP(*(.sa6_bl336src_addr))
		. = 0x00000080;
		KEEP(*(.sa6_bl336partition))
		. = 0x00000088;
		KEEP(*(.sa6_bl337src_addr))
		. = 0x00000090;
		KEEP(*(.sa6_bl337partition))
		. = 0x00000098;
		KEEP(*(.sa6_bl338src_addr))
		. = 0x000000A0;
		KEEP(*(.sa6_bl338partition))
		. = 0x00000554;
		KEEP(*(.sa6_bl31dst_addr))
		. = 0x00000558;
		KEEP(*(.sa6_bl31dst_addrh))
		. = 0x00000664;
		KEEP(*(.sa6_bl31dst_size))
		. = 0x00000D54;
		KEEP(*(.sa6_bl32dst_addr))
		. = 0x00000D58;
		KEEP(*(.sa6_bl32dst_addrh))
		. = 0x00000E64;
		KEEP(*(.sa6_bl32dst_size))
		. = 0x00001554;
		KEEP(*(.sa6_bl33dst_addr))
		. = 0x00001558;
		KEEP(*(.sa6_bl33dst_addrh))
		. = 0x00001664;
		KEEP(*(.sa6_bl33dst_size))
		. = 0x00001D54;
		KEEP(*(.sa6_bl332dst_addr))
		. = 0x00001D58;
		KEEP(*(.sa6_bl332dst_addrh))
		. = 0x00001E64;
		KEEP(*(.sa6_bl332dst_size))
		. = 0x00002554;
		KEEP(*(.sa6_bl333dst_addr))
		. = 0x00002558;
		KEEP(*(.sa6_bl333dst_addrh))
		. = 0x00002664;
		KEEP(*(.sa6_bl333dst_size))
		. = 0x00002D54;
		KEEP(*(.sa6_bl334dst_addr))
		. = 0x00002D58;
		KEEP(*(.sa6_bl334dst_addrh))
		. = 0x00002E64;
		KEEP(*(.sa6_bl334dst_size))
		. = 0x00003554;
		KEEP(*(.sa6_bl335dst_addr))
		. = 0x00003558;
		KEEP(*(.sa6_bl335dst_addrh))
		. = 0x00003664;
		KEEP(*(.sa6_bl335dst_size))
		. = 0x00003D54;
		KEEP(*(.sa6_bl336dst_addr))
		. = 0x00003D58;
		KEEP(*(.sa6_bl336dst_addrh))
		. = 0x00003E64;
		KEEP(*(.sa6_bl336dst_size))
		. = 0x00004554;
		KEEP(*(.sa6_bl337dst_addr))
		. = 0x00004558;
		KEEP(*(.sa6_bl337dst_addrh))
		. = 0x00004664;
		KEEP(*(.sa6_bl337dst_size))
		. = 0x00004D54;
		KEEP(*(.sa6_bl338dst_addr))
		. = 0x00004D58;
		KEEP(*(.sa6_bl338dst_addrh))
		. = 0x00004E64;
		KEEP(*(.sa6_bl338dst_size))
	}

}

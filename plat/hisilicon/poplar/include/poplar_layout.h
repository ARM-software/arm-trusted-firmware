/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __POPLAR_LAYOUT_H
#define __POPLAR_LAYOUT_H

/*
 * Boot memory layout definitions for the HiSilicon Poplar board
 */

/*
 * When Poplar is powered on, boot ROM loads the initial content of
 * boot media into low memory, verifies it, and begins executing it
 * in 32-bit mode.  The image loaded is "l-loader.bin", which contains
 * a small amount code along with an embedded ARM Trusted Firmware
 * BL1 image.  The main purpose of "l-loader" is to prepare the
 * processor to execute the BL1 image in 64-bit mode, and to trigger
 * that execution.
 *
 * Also embedded in "l-loader.bin" is a FIP image that contains
 * other ARM Trusted Firmware images:  BL2; BL31; and for BL33,
 * U-Boot.  When BL1 executes, it unpacks the BL2 image from the FIP
 * image into a region of memory set aside to hold it.  Similarly,
 * BL2 unpacks BL31 into memory reserved for it, and unpacks U-Boot
 * into high memory.
 *
 * Because the BL1 code is embedded in "l-loader", its base address
 * in memory is derived from the base address of the "l-loader"
 * text section, together with an offset.  Memory space for BL2 is
 * reserved immediately following BL1, and memory space is reserved
 * for BL31 after that.  ARM Trusted Firmware requires each of these
 * memory regions to be aligned on page boundaries, so the size of
 * each region is a multiple of a page size (ending in 000).  Note
 * that ARM Trusted Firmware requires the read-only and read-write
 * regions of memory used for BL1 to be defined separately.
 *
 *    ---------------------
 *    |  (unused memory)  |
 *    +-------------------+	- - - - -
 *    |  (l-loader text)  |               \
 *    +-------------------+                \
 *    |  BL1 (read-only)  | \               \
 *    |- - - - - - - - - -| |               |
 *    |  BL1 (read-write) | |               |
 *    +-------------------+  >  BL Memory   |
 *    |  Reserved for BL2 | |                > "l-loader.bin" image
 *    +-------------------+ |               |
 *    | Reserved for BL31 | /               |
 *    +-------------------+                 |
 *           . . .                          /
 *    +-------------------+                /
 *    |        FIP        |               /
 *    +-------------------+	- - - - -
 *           . . .
 *    |  (unused memory)  |
 *           . . .
 *    +-------------------+
 *    |Reserved for U-Boot|
 *    +-------------------+
 *           . . .
 *    |  (unused memory)  |
 *    ---------------------
 *
 * The size of each of these regions is defined below.  The base
 * address of the "l-loader" TEXT section and the offset of the BL1
 * image within that serve as anchors for defining the positions of
 * all other regions.  The FIP is placed in a section of its own.
 *
 * A "BASE" is the memory address of the start of a region; a "LIMIT"
 * marks its end.  A "SIZE" is the size of a region (in bytes).  An
 * "OFFSET" is an offset to the start of a region relative to the
 * base of the "l-loader" TEXT section (also a multiple of page size).
 */
#define LLOADER_TEXT_BASE		0x02001000	/* page aligned */
#define BL1_OFFSET			0x0000D000	/* page multiple */
#define FIP_BASE			0x02040000

#define BL1_RO_SIZE			0x00008000	/* page multiple */
#define BL1_RW_SIZE			0x00008000	/* page multiple */
#define BL1_SIZE			(BL1_RO_SIZE + BL1_RW_SIZE)
#define BL2_SIZE			0x0000c000	/* page multiple */
#define BL31_SIZE			0x00014000
#define FIP_SIZE			0x000c0000  /* absolute max */

     /* BL1_OFFSET */			/* (Defined above) */
#define BL1_BASE			(LLOADER_TEXT_BASE + BL1_OFFSET)
#define BL1_LIMIT			(BL1_BASE + BL1_SIZE)

#define BL1_RO_OFFSET			(BL1_OFFSET)
#define BL1_RO_BASE			(LLOADER_TEXT_BASE + BL1_RO_OFFSET)
#define BL1_RO_LIMIT			(BL1_RO_BASE + BL1_RO_SIZE)

#define BL1_RW_OFFSET			(BL1_RO_OFFSET + BL1_RO_SIZE)
#define BL1_RW_BASE			(LLOADER_TEXT_BASE + BL1_RW_OFFSET)
#define BL1_RW_LIMIT			(BL1_RW_BASE + BL1_RW_SIZE)

#define BL2_OFFSET			(BL1_OFFSET + BL1_SIZE)
#define BL2_BASE			(LLOADER_TEXT_BASE + BL2_OFFSET)
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define BL31_OFFSET			(BL2_OFFSET + BL2_SIZE)
#define BL31_BASE			(LLOADER_TEXT_BASE + BL31_OFFSET)
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#endif /* !__POPLAR_LAYOUT_H */

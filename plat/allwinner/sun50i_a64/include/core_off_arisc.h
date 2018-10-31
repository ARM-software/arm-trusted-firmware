/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

static uint32_t arisc_core_off[] = {
	0x18600000, /* l.movhi	r3, <corenr>	*/
	0x18000000, /* l.movhi	r0, 0x0		*/
	0x19a00170, /* l.movhi	r13, 0x170	*/
	0x84ad0030, /* l.lwz	r5, 0x30(r13)	*/
	0xe0a51803, /* l.and	r5, r5, r3	*/
	0xe4050000, /* l.sfeq	r5, r0		*/
	0x13fffffd, /* l.bf	-12		*/

	0xb8c30050, /* l.srli	r6, r3, 16	*/
	0xbc060001, /* l.sfeqi	r6, 1		*/
	0x10000005, /* l.bf	+20		*/
	0x19a001f0, /* l.movhi	r13, 0x1f0	*/
	0x84ad1500, /* l.lwz	r5, 0x1500(r13)	*/
	0xe0a53004, /* l.or	r5, r5, r6	*/
	0xd44d2d00, /* l.sw	0x1500(r13), r5	*/

	0x84ad1c30, /* l.lwz	r5, 0x1c30(r13)	*/
	0xacc6ffff, /* l.xori	r6, r6, -1	*/
	0xe0a53003, /* l.and	r5, r5, r6	*/
	0xd46d2c30, /* l.sw	0x1c30(r13), r5	*/

	0xe0c3000f, /* l.ff1	r6, r3		*/
	0x9cc6ffef, /* l.addi	r6, r6, -17	*/
	0xb8c60002, /* l.slli	r6, r6, 2	*/
	0xe0c66800, /* l.add	r6, r6, r13	*/
	0xa8a000ff, /* l.ori	r5, r0, 0xff	*/
	0xd4462d40, /* l.sw	0x1540(r6), r5	*/

	0xd46d0400, /* l.sw	0x1c00(r13), r0	*/
	0x03ffffff, /* l.j	-1		*/
	0x15000000, /* l.nop			*/
};

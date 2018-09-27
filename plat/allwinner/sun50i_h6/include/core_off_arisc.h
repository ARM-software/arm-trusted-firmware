/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

static uint32_t arisc_core_off[] = {
	0x18600000, /* l.movhi	r3, <corenr>	*/
	0x18000000, /* l.movhi	r0, 0x0		*/
	0x19a00901, /* l.movhi	r13, 0x901	*/
	0x84ad0080, /* l.lwz	r5, 0x80(r13)	*/
	0xe0a51803, /* l.and	r5, r5, r3	*/
	0xe4050000, /* l.sfeq	r5, r0		*/
	0x13fffffd, /* l.bf	-12		*/
	0xb8c30050, /* l.srli	r6, r3, 16	*/

	0xbc060001, /* l.sfeqi	r6, 1		*/
	0x10000005, /* l.bf	+20		*/
	0x19a00700, /* l.movhi	r13, 0x700	*/
	0x84ad0444, /* l.lwz	r5, 0x0444(r13)	*/
	0xe0a53004, /* l.or	r5, r5, r6	*/
	0xd40d2c44, /* l.sw	0x0444(r13), r5	*/

	0x84ad0440, /* l.lwz	r5, 0x0440(r13)	*/
	0xacc6ffff, /* l.xori	r6, r6, -1	*/
	0xe0a53003, /* l.and	r5, r5, r6	*/
	0xd40d2c40, /* l.sw	0x0440(r13), r5	*/

	0xe0c3000f, /* l.ff1	r6, r3		*/
	0x9cc6ffef, /* l.addi	r6, r6, -17	*/
	0xb8c60002, /* l.slli	r6, r6, 2	*/
	0xe0c66800, /* l.add	r6, r6, r13	*/
	0xa8a000ff, /* l.ori	r5, r0, 0xff	*/
	0xd4062c50, /* l.sw	0x0450(r6), r5	*/

	0xd40d0400, /* l.sw	0x0400(r13), r0	*/
	0x03ffffff, /* l.j	-1		*/
	0x15000000, /* l.nop			*/
};

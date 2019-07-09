/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
	.data
	.globl	jmptbl
	.align	4
jmptbl:	.quad	0x${jmptbl_address}

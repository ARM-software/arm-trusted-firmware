/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TZC380_H_
#define _TZC380_H_

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	unsigned int low_addr;
	unsigned int high_addr;
	unsigned int size;
	unsigned int sub_mask;
};

#endif /* _TZC380_H_ */

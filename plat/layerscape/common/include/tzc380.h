/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC380_H
#define TZC380_H

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	unsigned int low_addr;
	unsigned int high_addr;
	unsigned int size;
	unsigned int sub_mask;
};

#endif /* TZC380_H */

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERRUPT_PROPS_H
#define INTERRUPT_PROPS_H

#ifndef __ASSEMBLY__

/* Create an interrupt property descriptor from various interrupt properties */
#define INTR_PROP_DESC(num, pri, grp, cfg) \
	{ \
		.intr_num = (num), \
		.intr_pri = (pri), \
		.intr_grp = (grp), \
		.intr_cfg = (cfg), \
	}

typedef struct interrupt_prop {
	unsigned int intr_num:10;
	unsigned int intr_pri:8;
	unsigned int intr_grp:2;
	unsigned int intr_cfg:2;
} interrupt_prop_t;

#endif /* __ASSEMBLY__ */
#endif /* INTERRUPT_PROPS_H */

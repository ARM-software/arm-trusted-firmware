/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AMU_H__
#define __AMU_H__

/* Enable all group 0 counters */
#define AMU_GROUP0_COUNTERS_MASK	0xf

void amu_enable(int el2_unused);

#endif /* __AMU_H__ */

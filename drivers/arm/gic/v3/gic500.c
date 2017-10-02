/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for GIC500-specific features. This driver only overrides APIs that are
 * different to those generic ones in GICv3 driver.
 */
#include "gicv3_private.h"

void gicv3_distif_pre_save(unsigned int proc_num)
{
	arm_gicv3_distif_pre_save(proc_num);
}

void gicv3_distif_post_restore(unsigned int proc_num)
{
	arm_gicv3_distif_post_restore(proc_num);
}


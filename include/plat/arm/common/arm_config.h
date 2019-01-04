/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_CONFIG_H
#define ARM_CONFIG_H

#include <stdint.h>

#include <lib/utils_def.h>

/* Whether Base memory map is in use */
#define ARM_CONFIG_BASE_MMAP		BIT(1)

/* Whether TZC should be configured */
#define ARM_CONFIG_HAS_TZC		BIT(2)

/* FVP model has shifted affinity */
#define ARM_CONFIG_FVP_SHIFTED_AFF	BIT(3)

/* FVP model has SMMUv3 affinity */
#define ARM_CONFIG_FVP_HAS_SMMUV3	BIT(4)

/* FVP model has CCI (400 or 500/550) devices */
#define ARM_CONFIG_FVP_HAS_CCI400	BIT(5)
#define ARM_CONFIG_FVP_HAS_CCI5XX	BIT(6)

typedef struct arm_config {
	unsigned long flags;
} arm_config_t;


/* If used, arm_config must be defined and populated in the platform port */
extern arm_config_t arm_config;

static inline const arm_config_t *get_arm_config(void)
{
	return &arm_config;
}


#endif /* ARM_CONFIG_H */

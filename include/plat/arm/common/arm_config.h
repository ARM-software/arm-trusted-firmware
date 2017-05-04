/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_CONFIG_H__
#define __ARM_CONFIG_H__

#include <stdint.h>

enum arm_config_flags {
	/* Whether Base memory map is in use */
	ARM_CONFIG_BASE_MMAP		= 0x1,
	/* Whether interconnect should be enabled */
	ARM_CONFIG_HAS_INTERCONNECT	= 0x2,
	/* Whether TZC should be configured */
	ARM_CONFIG_HAS_TZC		= 0x4
};

typedef struct arm_config {
	unsigned long flags;
} arm_config_t;


/* If used, arm_config must be defined and populated in the platform port */
extern arm_config_t arm_config;

static inline const arm_config_t *get_arm_config(void)
{
	return &arm_config;
}


#endif /* __ARM_CONFIG_H__ */

/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>

#define ISOLATE_FIELD(reg, feat)					\
	((unsigned int)(((reg) >> (feat ## _SHIFT)) & (feat ## _MASK)))

static inline bool is_armv7_gentimer_present(void)
{
	return ISOLATE_FIELD(read_id_pfr1(), ID_PFR1_GENTIMER) != 0U;
}

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ISOLATE_FIELD(read_id_mmfr4(), ID_MMFR4_CNP) != 0U;
}

#endif /* ARCH_FEATURES_H */

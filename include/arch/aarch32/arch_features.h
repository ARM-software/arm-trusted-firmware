/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ((read_id_mmfr4() >> ID_MMFR4_CNP_SHIFT) &
		ID_MMFR4_CNP_MASK) != 0U;
}

#endif /* ARCH_FEATURES_H */

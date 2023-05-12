/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Header file to contain common macros across different platforms */
#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

#define __bf_shf(x)            (__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

#endif /* PLAT_COMMON_H */

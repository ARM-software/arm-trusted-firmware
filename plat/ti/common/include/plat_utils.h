/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_PRIVATE_UTILS_H
#define PLATFORM_PRIVATE_UTILS_H

#include <lib/mmio.h>

#define __bf_shf(x) (__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)	\
	({ \
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask)); \
	})

#define FIELD_PREP(_mask, _val) \
	({ \
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask); \
	})
#endif /* PLATFORM_PRIVATE_UTILS_H */

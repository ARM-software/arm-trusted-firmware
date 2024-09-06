/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSREG128_H
#define SYSREG128_H

#ifndef __ASSEMBLER__

#if ENABLE_FEAT_D128
#include <stdint.h>

typedef uint128_t sysreg_t;

#define PAR_EL1_D128	(((sysreg_t)(1ULL)) << (64))

#define _DECLARE_SYSREG128_READ_FUNC(_name)	\
uint128_t read_ ## _name(void);

#define _DECLARE_SYSREG128_WRITE_FUNC(_name)	\
void write_ ## _name(uint128_t v);

#define DECLARE_SYSREG128_RW_FUNCS(_name)	\
	_DECLARE_SYSREG128_READ_FUNC(_name)	\
	_DECLARE_SYSREG128_WRITE_FUNC(_name)
#else

typedef uint64_t sysreg_t;

#endif /* ENABLE_FEAT_D128 */

#endif /* __ASSEMBLER__ */

#endif /* SYSREG128_H */

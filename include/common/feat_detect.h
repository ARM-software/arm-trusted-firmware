/*
 * Copyright (c) 2022-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT_H
#define FEAT_DETECT_H

/* Function Prototypes */
void detect_arch_features(unsigned int core_pos);

/* Macro Definitions */
#define FEAT_STATE_DISABLED		0
#define FEAT_STATE_ALWAYS		1
#define FEAT_STATE_CHECKED		2

#define _CREATE_FEATURE_PRESENT(name, idreg, idfield, idval)			\
__attribute__((always_inline))							\
static inline unsigned int read_ ## name ## _id_field(void)			\
{										\
	return (unsigned int)EXTRACT(idfield, read_ ## idreg());		\
}										\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _present(void)				\
{										\
	return (read_ ## name ## _id_field() >= idval) ? true : false; 		\
}

#define CREATE_FEATURE_SUPPORTED(name, read_func, guard)			\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _supported(void)				\
{										\
	if ((guard) == FEAT_STATE_DISABLED) {					\
		return false;							\
	}									\
	if ((guard) == FEAT_STATE_ALWAYS) {					\
		return true;							\
	}									\
	return read_func();							\
}

#endif /* FEAT_DETECT_H */

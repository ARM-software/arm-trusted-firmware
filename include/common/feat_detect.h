/*
 * Copyright (c) 2022-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT_H
#define FEAT_DETECT_H

/* Function Prototypes */
void detect_arch_features(unsigned int core_pos);

/*
 * The `ENABLE_FEAT` mechanism is implemented with a tri-state approach for each
 * feature:
 * ENABLE_FEAT_xxx = 0 : The feature is disabled statically at compile time.
 * ENABLE_FEAT_xxx = 1 : The feature is enabled and must be present in hardware.
 *                       `FEATURE_DETECTION` will panic if feature is not present
 *                       at cold boot.
 * ENABLE_FEAT_xxx = 2 : The feature is enabled but dynamically enabled at
 *                       runtime depending on hardware capability.
 *
 *  For better readability, state values are defined with macros:
 */
#define FEAT_STATE_DISABLED		0
#define FEAT_STATE_ALWAYS		1
#define FEAT_STATE_CHECKED		2

/*
 * Force inlining to let the compiler optimise away the whole check on
 * FEAT_STATE_DISABLED.
 */
#define _CREATE_FEATURE_ID_FIELD(name, idreg, guard, field, min, max, worlds)	\
__attribute__((always_inline))							\
static inline unsigned int _read_ ## name ## _id_field(void)			\
{										\
	return (unsigned int)EXTRACT(field, read_ ## idreg());			\
}

#define _CREATE_FEATURE_PRESENT(name, idreg, guard, field, min, max, worlds)	\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _present(void)				\
{										\
	return (_read_ ## name ## _id_field() >= min) ? true : false;		\
}

#define _CREATE_FEATURE_SUPPORTED(name, idreg, guard, field, min, max, worlds)	\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _supported(void)				\
{										\
	if ((guard) == FEAT_STATE_DISABLED) {					\
		return false;							\
	}									\
	if ((guard) == FEAT_STATE_ALWAYS) {					\
		return true;							\
	}									\
	return is_ ## name ## _present();					\
}

#endif /* FEAT_DETECT_H */

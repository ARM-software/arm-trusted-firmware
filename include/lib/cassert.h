/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CASSERT_H
#define CASSERT_H

#include <cdefs.h>

/*******************************************************************************
 * Macro to flag a compile time assertion. It uses the preprocessor to generate
 * an invalid C construct if 'cond' evaluates to false.
 * The following compilation error is triggered if the assertion fails:
 * "error: size of array 'msg' is negative"
 * The 'unused' attribute ensures that the unused typedef does not emit a
 * compiler warning.
 ******************************************************************************/
#define CASSERT(cond, msg)	\
	typedef char msg[(cond) ? 1 : -1] __unused

#endif /* CASSERT_H */

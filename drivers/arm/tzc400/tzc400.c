/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if PLAT_COMPAT >= 201601
#error "Using deprecated TZC-400 source file"
#else
#include "../tzc/tzc400.c"
#endif /* PLAT_COMPAT >= 201601 */

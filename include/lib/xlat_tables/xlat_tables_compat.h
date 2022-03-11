/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_COMPAT_H
#define XLAT_TABLES_COMPAT_H

#if XLAT_TABLES_LIB_V2
#include <lib/xlat_tables/xlat_tables_v2.h>
#else
#include <lib/xlat_tables/xlat_tables.h>
#endif

#endif /* XLAT_TABLES_COMPAT_H */

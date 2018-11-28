/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if MULTI_CONSOLE_API
#if ERROR_DEPRECATED
#error "console.S is deprecated, platforms should no longer link it explicitly"
#endif
#else
#include "deprecated_console.S"
#endif

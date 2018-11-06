/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Build platform specific handling.
 * This allows for builds on non-Posix platforms
 * e.g. Visual Studio on Windows
 */

#ifndef FIPTOOL_PLATFORM_H
#define FIPTOOL_PLATFORM_H

#ifndef _MSC_VER

/* Not Visual Studio, so include Posix Headers. */
# include <getopt.h>
# include <openssl/sha.h>
# include <unistd.h>

# define  BLD_PLAT_STAT stat

#else

/* Visual Studio. */
# include "win_posix.h"

#endif

#endif /* FIPTOOL_PLATFORM_H */

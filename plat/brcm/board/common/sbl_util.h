/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_UTIL_H
#define SBL_UTIL_H

#include <stdint.h>

#include <sotp.h>

#define SBL_DISABLED 0
#define SBL_ENABLED 1

int sbl_status(void);

#endif /* #ifdef SBL_UTIL_H */

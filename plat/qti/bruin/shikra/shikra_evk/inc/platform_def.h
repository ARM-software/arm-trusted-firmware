/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <shikra_def.h>

#define BL31_BASE			0xA1300000
#define BL31_SIZE			0x000C8000	/* 800KB */
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#endif /* PLATFORM_DEF_H */

/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_TRP_H
#define PLATFORM_TRP_H

#include <services/rmm_core_manifest.h>

/*******************************************************************************
 * Mandatory TRP functions (only if platform contains a TRP)
 ******************************************************************************/
void trp_early_platform_setup(rmm_manifest_t *manifest);

#endif /* PLATFORM_TRP_H */

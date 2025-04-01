/*
 * Copyright (c) 2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_OCM_COHERENCY_H
#define PLAT_OCM_COHERENCY_H

#define COHERENCY_CHECK_NOT_SUPPORTED	-1

#if (DEBUG == 1)
int32_t check_ocm_coherency(void);
#else
static inline int32_t check_ocm_coherency(void)
{
	return COHERENCY_CHECK_NOT_SUPPORTED;
}
#endif

#endif/*PLAT_OCM_COHERENCY_H*/

/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DFD_H
#define DFD_H

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <plat_dfd.h>

#if MTK_DFD_SUPPORT
void dfd_resume(void);
void dfd_setup(uint64_t base_addr, uint64_t chain_length, uint64_t cache_dump);
uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			    uint64_t arg2, uint64_t arg3);
#else
#include <mtk_sip_svc.h>
static inline void dfd_resume(void) {}
static inline void dfd_setup(uint64_t base_addr, uint64_t chain_length, uint64_t cache_dump) {}
static inline uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
					    uint64_t arg2, uint64_t arg3)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}
#endif

#endif /* DFD_H */

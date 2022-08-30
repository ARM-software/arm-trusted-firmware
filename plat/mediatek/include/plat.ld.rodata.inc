/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LD_RODATA_INC
#define PLAT_LD_RODATA_INC

#include <lib/mtk_init/mtk_init.h>
	. = ALIGN(32);
	INIT_CALL_TABLE(EXPAND_AS_LINK_SECTION);
	__MTK_PLAT_INITCALL_END__ = .;
	. = ALIGN(32);
	__MTK_MMAP_POINTER_POOL_START__ = .;
	KEEP(*(.mtk_mmap_pool))
	__MTK_MMAP_POINTER_POOL_END_UNALIGNED__ = .;
	. = ALIGN(8);
	__MTK_MMAP_POOL_START__ = .;
	KEEP(*(.mtk_mmap_lists))
	__MTK_MMAP_POOL_END_UNALIGNED__ = .;
	. = ALIGN(32);
	__MTK_SMC_POOL_START__ = .;
	KEEP(*(.mtk_smc_descriptor_pool))
	__MTK_SMC_POOL_END_UNALIGNED__ = .;
	. = ALIGN(8);
#include <vendor_pubsub_events.h>
	*(mtk_plat_ro)

#endif /* PLAT_LD_RODATA_INC */

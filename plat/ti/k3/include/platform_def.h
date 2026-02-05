/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <ti_platform_defs.h>

#if !K3_SEC_PROXY_LITE
#define SEC_PROXY_DATA_BASE	0x32C00000
#define SEC_PROXY_DATA_SIZE	0x80000
#define SEC_PROXY_SCFG_BASE	0x32800000
#define SEC_PROXY_SCFG_SIZE	0x80000
#define SEC_PROXY_RT_BASE	0x32400000
#define SEC_PROXY_RT_SIZE	0x80000
#else
#define SEC_PROXY_DATA_BASE	0x4D000000
#define SEC_PROXY_DATA_SIZE	0x80000
#define SEC_PROXY_SCFG_BASE	0x4A400000
#define SEC_PROXY_SCFG_SIZE	0x80000
#define SEC_PROXY_RT_BASE	0x4A600000
#define SEC_PROXY_RT_SIZE	0x80000
#endif /* K3_SEC_PROXY_LITE */

#define SEC_PROXY_TIMEOUT_US		1000000
#define SEC_PROXY_MAX_MESSAGE_SIZE	56

/*******************************************************************************
 * Memory layout constants
 ******************************************************************************/

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 *
 * The link addresses are determined by BL31_BASE + offset.
 * When ENABLE_PIE is set, the TF images can be loaded anywhere, so
 * BL31_BASE is really arbitrary.
 *
 * When ENABLE_PIE is unset, BL31_BASE should be chosen so that
 * it matches to the physical address where BL31 is loaded, that is,
 * BL31_BASE should be the base address of the RAM region.
 *
 * Lets make things explicit by mapping BL31_BASE to 0x0 since ENABLE_PIE is
 * defined as default for our platform.
 */
#define BL31_BASE	UL(0x00000000) /* PIE remapped on fly */
#define BL31_SIZE	UL(0x00040000) /* 128k */
#define BL31_LIMIT	(BL31_BASE + BL31_SIZE)

#endif /* PLATFORM_DEF_H */

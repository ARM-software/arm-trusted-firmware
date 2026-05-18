/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_ARM_LFA_COMPONENTS_H
#define PLAT_ARM_LFA_COMPONENTS_H

/*
 * Staging area for hosting Secure Partition packages intended for live
 * activation.
 */
#define FVP_SP1_STAGING_MEM_BASE	0x7000000U
#define FVP_SP2_STAGING_MEM_BASE	0x7100000U

#define FFA_ID_SP1		0x8001
#define FFA_ID_SP2		0x8002

#define IMAGE_UUID_SECURE_PARTITION_1					\
	{ { 0xf0, 0x7b, 0x2a, 0x96 }, { 0x1d, 0x47 }, { 0x4d, 0x17 },	\
	0x9e, 0xc8, { 0x86, 0xa6, 0x4e, 0x25, 0x3e, 0x5c } }

#define IMAGE_UUID_SECURE_PARTITION_2					\
	{ { 0xc3, 0xff, 0x21, 0x27 }, { 0x7e, 0x41 }, { 0xa9, 0xf8 },	\
	0x05, 0xaf, { 0x24, 0xa1, 0xaf, 0xa3, 0x34, 0x74 } }

/*
 * Define platform-specific numeric IDs for LFA FVP components.
 */
typedef enum {
#if ENABLE_LFA_BL31
	LFA_BL31_COMPONENT,
#endif
#if BL32_BASE
	LFA_BL32_COMPONENT,
#endif /* BL32_BASE */
	LFA_BL33_COMPONENT,
#if ENABLE_RMM
	LFA_RMM_COMPONENT,
#endif /* ENABLE_RMM */
#if SUPPORT_SP_LIVE_ACTIVATION
	LFA_SP1,
	LFA_SP2,
#endif
	LFA_MAX_DEFINED_COMPONENTS
} fvp_lfa_component_id_t;

#endif /* PLAT_ARM_LFA_COMPONENTS_H */

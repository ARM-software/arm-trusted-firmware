/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_ARM_LFA_COMPONENTS_H
#define PLAT_ARM_LFA_COMPONENTS_H

/*
 * Define platform-specific numeric IDs for LFA FVP components.
 */
typedef enum {
	LFA_BL31_COMPONENT = 0,
#if BL32_BASE
	LFA_BL32_COMPONENT,
#endif /* BL32_BASE */
	LFA_BL33_COMPONENT,
#if ENABLE_RME
	LFA_RMM_COMPONENT,
#endif /* ENABLE_RME */
	LFA_MAX_DEFINED_COMPONENTS
} fvp_lfa_component_id_t;

#endif /* PLAT_ARM_LFA_COMPONENTS_H */

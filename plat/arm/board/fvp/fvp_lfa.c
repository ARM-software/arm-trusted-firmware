/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/rmmd_rmm_lfa.h>
#include <tools_share/firmware_image_package.h>

#include <fvp_lfa_components.h>

/* Keep this array consistent with enum fvp_lfa_component_id_t */
static plat_lfa_component_info_t fvp_lfa_components[LFA_MAX_DEFINED_COMPONENTS] = {
	[LFA_BL31_COMPONENT] = {LFA_BL31_COMPONENT, UUID_EL3_RUNTIME_FIRMWARE_BL31,
				NULL, false},
#if BL32_BASE
	[LFA_BL32_COMPONENT] = {LFA_BL32_COMPONENT, UUID_SECURE_PAYLOAD_BL32,
				NULL, false},
#endif /* BL32_BASE */
	[LFA_BL33_COMPONENT] = {LFA_BL33_COMPONENT, UUID_NON_TRUSTED_FIRMWARE_BL33,
				NULL, false},
#if ENABLE_RME
	[LFA_RMM_COMPONENT]  = {LFA_RMM_COMPONENT, UUID_REALM_MONITOR_MGMT_FIRMWARE,
				NULL, false},
#endif /* ENABLE_RME */
};

uint32_t plat_lfa_get_components(plat_lfa_component_info_t **components)
{
	if (components == NULL) {
		return -EINVAL;
	}

	fvp_lfa_components[LFA_BL31_COMPONENT].activator = get_bl31_activator();
#if ENABLE_RME
	fvp_lfa_components[LFA_RMM_COMPONENT].activator = get_rmm_activator();
#endif /* ENABLE_RME */

	*components = fvp_lfa_components;
	return LFA_MAX_DEFINED_COMPONENTS;
}

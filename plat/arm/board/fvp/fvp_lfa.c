/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <tools_share/firmware_image_package.h>

#include <plat/arm/common/plat_arm_lfa_components.h>

/* Keep this array consistent with enum fvp_lfa_component_id_t */
static plat_lfa_component_info_t fvp_lfa_components[LFA_MAX_DEFINED_COMPONENTS] = {

#if ENABLE_LFA_BL31
	[LFA_BL31_COMPONENT] = {LFA_BL31_COMPONENT, UUID_EL3_RUNTIME_FIRMWARE_BL31,
				NULL, false},
#endif /* ENABLE_LFA_BL31 */

#if BL32_BASE
	[LFA_BL32_COMPONENT] = {LFA_BL32_COMPONENT, UUID_SECURE_PAYLOAD_BL32,
				NULL, false},
#endif /* BL32_BASE */

	[LFA_BL33_COMPONENT] = {LFA_BL33_COMPONENT, UUID_NON_TRUSTED_FIRMWARE_BL33,
				NULL, false},

#if ENABLE_RMM
	[LFA_RMM_COMPONENT]  = {LFA_RMM_COMPONENT, UUID_REALM_MONITOR_MGMT_FIRMWARE,
				NULL, false},

#endif /* ENABLE_RMM */

#if SUPPORT_SP_LIVE_ACTIVATION
	[LFA_SP1] = { LFA_SP1, IMAGE_UUID_SECURE_PARTITION_1, NULL, false},

	[LFA_SP2] = { LFA_SP2, IMAGE_UUID_SECURE_PARTITION_2, NULL, false},
#endif /* SUPPORT_SP_LIVE_ACTIVATION */

};

uint32_t plat_lfa_get_components(plat_lfa_component_info_t **components)
{
	if (components == NULL) {
		return -EINVAL;
	}

#if ENABLE_LFA_BL31
	fvp_lfa_components[LFA_BL31_COMPONENT].activator = get_bl31_activator();
#endif /* ENABLE_LFA_BL31 */

#if ENABLE_RMM
	fvp_lfa_components[LFA_RMM_COMPONENT].activator = get_rmm_activator();
#endif /* ENABLE_RMM */

#if SUPPORT_SP_LIVE_ACTIVATION
	fvp_lfa_components[LFA_SP1].activator =
		get_secure_partition_activator();

	fvp_lfa_components[LFA_SP2].activator =
		get_secure_partition_activator();
#endif /* SUPPORT_SP_LIVE_ACTIVATION */

	*components = fvp_lfa_components;
	return LFA_MAX_DEFINED_COMPONENTS;
}

bool is_plat_lfa_activation_pending(uint32_t lfa_component_id)
{
#if ENABLE_RMM
	if (lfa_component_id == LFA_RMM_COMPONENT) {
		return true;
	}
#endif /* ENABLE_RMM */

#if SUPPORT_SP_LIVE_ACTIVATION
	if (lfa_component_id == LFA_SP1 || lfa_component_id == LFA_SP2) {
		return true;
	}
#endif /* SUPPORT_SP_LIVE_ACTIVATION */

#if ENABLE_LFA_BL31
	if (lfa_component_id == LFA_BL31_COMPONENT) {
		return true;
	}
#endif /* ENABLE_LFA_BL31 */

	return false;
}

int plat_lfa_cancel(uint32_t lfa_component_id)
{
	/* placeholder function to do cancel LFA of given component */
	return 0;
}

int plat_lfa_load_auth_image(uint32_t lfa_component_id)
{
	return 0;
}

int plat_lfa_get_image_info(uint32_t lfa_component_id, uintptr_t *image_address,
			    size_t *image_size)
{
#if ENABLE_LFA_BL31
	/*
	 * In AEM FVP, we don't want to bloat the code by adding
	 * loading and authentication mechanism, so here we assumed
	 * that the components are pre-loaded and authenticated already.
	 *
	 * The image is pre-loaded at 0xFB000000, size is not used for
	 * sectional copy.
	 */
	if (lfa_component_id == LFA_BL31_COMPONENT) {
		*image_size = 0;
		*image_address = (uintptr_t)(PLAT_LFA_STORE_BASE);
		VERBOSE("FVP BL31 LFA image at 0x%lX with size %ld bytes\n",
			(uint64_t)*image_address, *image_size);
		return 0;
	}
#endif

	return -EPERM;
}

int plat_lfa_notify_activate(uint32_t lfa_component_id)
{
	return 0;
}

uint64_t plat_lfa_mailbox_base(void)
{
	return PLAT_ARM_TRUSTED_MAILBOX_BASE;
}

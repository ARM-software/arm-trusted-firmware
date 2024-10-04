/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/dice_prot_env.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/tbbr_oid.h>

#include "tc_dpe.h"
#include <tc_rse_comms.h>

/*
 * The content and the values of this array depends on:
 * - build config: Which components are loaded: SPMD, TOS, SPx, etc ?
 * - boot order: the last element in a layer should be treated differently.
 */

/*
 * TODO:
 *     - The content of the array must be tailored according to the build
 *       config (TOS, SPMD, etc). All loaded components (executables and
 *       config blobs) must be present in this array.
 *     - Current content is according to the Trusty build config.
 */
struct dpe_metadata tc_dpe_metadata[] = {
	{
		.id = BL31_IMAGE_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL31_IMAGE_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = BL31_IMAGE_KEY_OID },
	{
		.id = BL32_IMAGE_ID,
		.cert_id =  DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL32_IMAGE_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = BL32_IMAGE_KEY_OID },
	{
		.id = BL33_IMAGE_ID,
		.cert_id = DPE_HYPERVISOR_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL33_IMAGE_STRING,
		.allow_new_context_to_derive = true,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_AP_NS,
		.pk_oid = BL33_IMAGE_KEY_OID },

	{
		.id = HW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_HW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = HW_CONFIG_KEY_OID },
	{
		.id = NT_FW_CONFIG_ID,
		.cert_id = DPE_HYPERVISOR_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_NT_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NT_FW_CONFIG_KEY_OID },
	{
		.id = SCP_BL2_IMAGE_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SCP_BL2_IMAGE_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = SCP_BL2_IMAGE_KEY_OID },
	{
		.id = SOC_FW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SOC_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = SOC_FW_CONFIG_KEY_OID },
	{
		.id = TOS_FW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_TOS_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = TOS_FW_CONFIG_KEY_OID },
#if defined(SPD_spmd)
	{
		.id = SP_PKG1_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP1_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG2_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP2_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG3_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP3_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG4_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP4_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG5_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP5_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG6_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP6_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG7_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP7_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },
	{
		.id = SP_PKG8_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP8_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = NULL },

#endif
	{
		.id = DPE_INVALID_ID }
};

/* Context handle is meant to be used by BL33. Sharing it via NT_FW_CONFIG */
static int new_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle, int *parent_ctx_handle)
{
	new_ctx_handle = *ctx_handle;

	/* Irrelevant in BL2 because cold restart resumes CPU in BL1 */
	(void)parent_ctx_handle;
}

void plat_dpe_get_context_handle(int *ctx_handle)
{
	int rc;

	rc = arm_get_tb_fw_info(ctx_handle);
	if (rc != 0) {
		ERROR("Unable to get DPE context handle from TB_FW_CONFIG\n");
		/*
		 * It is a fatal error because on FVP platform, BL2 software
		 * assumes that a valid DPE context_handle is passed through
		 * the DTB object by BL1.
		 */
		plat_panic_handler();
	}

	VERBOSE("Received DPE context handle: 0x%x\n", *ctx_handle);
}

void bl2_plat_mboot_init(void)
{
#if defined(SPD_spmd)
	size_t i;
	const size_t array_size = ARRAY_SIZE(tc_dpe_metadata);

	for (i = 0U; i < array_size; i++) {
		if (tc_dpe_metadata[i].id != SP_PKG1_ID) {
			continue;
		}

		if ((i + NUM_SP > array_size) || (i - 1 + NUM_SP < 0)) {
			ERROR("Secure partition number is out-of-range\n");
			ERROR("  Non-Secure partition number: %ld\n", i);
			ERROR("  Secure partition number: %d\n", NUM_SP);
			ERROR("  Metadata array size: %ld\n", array_size);
			panic();
		}

		/* Finalize the certificate on the last secure partition */
		tc_dpe_metadata[i - 1 + NUM_SP].create_certificate = true;
		break;
	}
#endif

	/* Initialize the communication channel between AP and RSE */
	(void)plat_rse_comms_init();

	dpe_init(tc_dpe_metadata);
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	VERBOSE("Share DPE context handle with BL33: 0x%x\n", new_ctx_handle);
	rc = arm_set_nt_fw_info(&new_ctx_handle);
	if (rc != 0) {
		ERROR("Unable to set DPE context handle in NT_FW_CONFIG\n");
		/*
		 * It is a fatal error because on TC platform, BL33 software
		 * assumes that a valid DPE context_handle is passed through
		 * the DTB object by BL2.
		 */
		plat_panic_handler();
	}
}

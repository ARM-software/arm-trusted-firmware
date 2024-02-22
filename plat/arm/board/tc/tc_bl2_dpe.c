/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/arm/rse_comms.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/dice_prot_env.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/tbbr_oid.h>

#include "tc_dpe_cert.h"

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
		.pk_oid = BL31_IMAGE_KEY_OID },
	{
		.id = BL32_IMAGE_ID,
		.cert_id =  DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL32_IMAGE_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = BL32_IMAGE_KEY_OID },
	{
		.id = BL33_IMAGE_ID,
		.cert_id = DPE_HYPERVISOR_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL33_IMAGE_STRING,
		.allow_new_context_to_derive = true,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = BL33_IMAGE_KEY_OID },

	{
		.id = HW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_HW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = HW_CONFIG_KEY_OID },
	{
		.id = NT_FW_CONFIG_ID,
		.cert_id = DPE_HYPERVISOR_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_NT_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NT_FW_CONFIG_KEY_OID },
	{
		.id = SCP_BL2_IMAGE_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SCP_BL2_IMAGE_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = SCP_BL2_IMAGE_KEY_OID },
	{
		.id = SOC_FW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SOC_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = SOC_FW_CONFIG_KEY_OID },
	{
		.id = TOS_FW_CONFIG_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_TOS_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = TOS_FW_CONFIG_KEY_OID },
#if defined(SPD_spmd)
	{
		.id = SP_PKG1_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP1_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = true, /* With Trusty only one SP is loaded */
		.pk_oid = NULL },
	{
		.id = SP_PKG2_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP2_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG3_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP3_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG4_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP4_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG5_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP5_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG6_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP6_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG7_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP7_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },
	{
		.id = SP_PKG8_ID,
		.cert_id = DPE_CERT_ID_SAME_AS_PARENT, /* AP_BL2: DPE_AP_FW_CERT_ID */
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SP8_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.pk_oid = NULL },

#endif
	{
		.id = DPE_INVALID_ID }
};

/* Context handle is meant to be used by BL33. Sharing it via NT_FW_CONFIG */
static int new_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle)
{
	new_ctx_handle = *ctx_handle;
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
	/* Initialize the communication channel between AP and RSE */
	(void)rse_comms_init(PLAT_RSE_AP_SND_MHU_BASE,
			     PLAT_RSE_AP_RCV_MHU_BASE);

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

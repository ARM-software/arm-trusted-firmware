/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/arm/css/sds.h>
#include <drivers/arm/rse_comms.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/dice_prot_env.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/zero_oid.h>

#include "tc_dpe.h"

struct dpe_metadata tc_dpe_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.cert_id = DPE_AP_FW_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = ZERO_OID },
	{
		.id = TB_FW_CONFIG_ID,
		.cert_id = DPE_AP_FW_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_TB_FW_CONFIG_STRING,
		.allow_new_context_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false,
		.target_locality = LOCALITY_NONE, /* won't derive don't care */
		.pk_oid = ZERO_OID },
	{
		.id = BL2_IMAGE_ID,
		.cert_id = DPE_AP_FW_CERT_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL2_IMAGE_STRING,
		.allow_new_context_to_derive = true,
		.retain_parent_context = true, /* To handle restart */
		.target_locality = LOCALITY_AP_S,
		.create_certificate = false,
		.pk_oid = ZERO_OID },
	{
		.id = DPE_INVALID_ID }
};

/* Effective timeout of 10000 ms */
#define RSE_DPE_BOOT_10US_RETRIES		1000000
#define TC2_SDS_DPE_CTX_HANDLE_STRUCT_ID	0x0000000A

/* Context handle is meant to be used by BL2. Sharing it via TB_FW_CONFIG */
static int new_ctx_handle;
/* Save a valid parent context handle to be able to send commands to DPE service
 * in case of an AP cold restart.
 */
static int new_parent_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle, int *parent_ctx_handle)
{
	new_ctx_handle = *ctx_handle;
	new_parent_ctx_handle = *parent_ctx_handle;
}

void plat_dpe_get_context_handle(int *ctx_handle)
{
	int retry = RSE_DPE_BOOT_10US_RETRIES;
	int ret;

	/* Initialize System level generic or SP804 timer */
	generic_delay_timer_init();

	/* Check the initialization of the Shared Data Storage area between RSE
	 * and AP. Since AP_BL1 is executed first then a bit later the RSE
	 * runtime, which initialize this area, therefore AP needs to check it
	 * in a loop until it gets written by RSE Secure Runtime.
	 */
	VERBOSE("Waiting for DPE service initialization in RSE Secure Runtime\n");
	while (retry > 0) {
		ret = sds_init(SDS_RSE_AP_REGION_ID);
		if (ret != SDS_OK) {
			udelay(10);
			retry--;
		} else {
			break;
		}
	}

	if (retry == 0) {
		ERROR("DPE init timeout\n");
		plat_panic_handler();
	} else {
		VERBOSE("DPE init succeeded in %dms.\n",
			(RSE_DPE_BOOT_10US_RETRIES - retry) / 100);
	}

	/* TODO: call this in a loop to avoid reading unfinished data */
	ret = sds_struct_read(SDS_RSE_AP_REGION_ID,
			      TC2_SDS_DPE_CTX_HANDLE_STRUCT_ID,
			      0,
			      ctx_handle,
			      sizeof(*ctx_handle),
			      SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Unable to get DPE context handle from SDS area\n");
		plat_panic_handler();
	}

	VERBOSE("Received DPE context handle: 0x%x\n", *ctx_handle);
}

void bl1_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSE */
	(void)rse_comms_init(PLAT_RSE_AP_SND_MHU_BASE,
			     PLAT_RSE_AP_RCV_MHU_BASE);

	dpe_init(tc_dpe_metadata);
}

void bl1_plat_mboot_finish(void)
{
	int rc;

	VERBOSE("Share DPE context handle with BL2: 0x%x\n", new_ctx_handle);
	rc = arm_set_tb_fw_info(&new_ctx_handle);
	if (rc != 0) {
		ERROR("Unable to set DPE context handle in TB_FW_CONFIG\n");
		/*
		 * It is a fatal error because on TC platform, BL2 software
		 * assumes that a valid DPE context_handle is passed through
		 * the DTB object by BL1.
		 */
		plat_panic_handler();
	}

	VERBOSE("Save parent context handle: 0x%x\n", new_parent_ctx_handle);
	rc = sds_struct_write(SDS_RSE_AP_REGION_ID,
			      TC2_SDS_DPE_CTX_HANDLE_STRUCT_ID,
			      0,
			      &new_parent_ctx_handle,
			      sizeof(new_parent_ctx_handle),
			      SDS_ACCESS_MODE_NON_CACHED);
	if (rc != SDS_OK) {
		ERROR("Unable to save DPE parent context handle to SDS area\n");
		plat_panic_handler();
	}
}

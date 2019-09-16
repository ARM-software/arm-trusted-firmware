/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <stdint.h>
#include <string.h>

#include "aml_private.h"

struct aml_cpu_info {
	uint32_t version;
	uint8_t chip_id[16];
};

static int aml_sip_get_chip_id(uint64_t version)
{
	struct aml_cpu_info *info = (void *)AML_SHARE_MEM_OUTPUT_BASE;
	uint32_t size;

	if (version > 2)
		return -1;

	memset(info, 0, sizeof(struct aml_cpu_info));

	if (version == 2) {
		info->version = 2;
		size = 16;
	} else {
		info->version = 1;
		size = 12;
	}

	if (aml_scpi_get_chip_id(info->chip_id, size) == 0)
		return -1;

	return 0;
}

/*******************************************************************************
 * This function is responsible for handling all SiP calls
 ******************************************************************************/
static uintptr_t aml_sip_handler(uint32_t smc_fid,
				  u_register_t x1, u_register_t x2,
				  u_register_t x3, u_register_t x4,
				  void *cookie, void *handle,
				  u_register_t flags)
{
	switch (smc_fid) {

	case AML_SM_GET_SHARE_MEM_INPUT_BASE:
		SMC_RET1(handle, AML_SHARE_MEM_INPUT_BASE);

	case AML_SM_GET_SHARE_MEM_OUTPUT_BASE:
		SMC_RET1(handle, AML_SHARE_MEM_OUTPUT_BASE);

	case AML_SM_EFUSE_READ:
	{
		void *dst = (void *)AML_SHARE_MEM_OUTPUT_BASE;
		uint64_t ret = aml_efuse_read(dst, (uint32_t)x1, x2);

		SMC_RET1(handle, ret);
	}
	case AML_SM_EFUSE_USER_MAX:
		SMC_RET1(handle,  aml_efuse_user_max());

	case AML_SM_JTAG_ON:
		aml_scpi_jtag_set_state(AML_JTAG_STATE_ON, x1);
		SMC_RET1(handle, 0);

	case AML_SM_JTAG_OFF:
		aml_scpi_jtag_set_state(AML_JTAG_STATE_OFF, x1);
		SMC_RET1(handle, 0);

	case AML_SM_GET_CHIP_ID:
		SMC_RET1(handle, aml_sip_get_chip_id(x1));

	default:
		ERROR("BL31: Unhandled SIP SMC: 0x%08x\n", smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

DECLARE_RT_SVC(
	aml_sip_handler,

	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	aml_sip_handler
);

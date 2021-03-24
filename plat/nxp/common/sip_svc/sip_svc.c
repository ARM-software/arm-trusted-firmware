/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>

#include <caam.h>
#include <common/runtime_svc.h>
#include <dcfg.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include <plat_common.h>
#include <sipsvc.h>

/* Layerscape SiP Service UUID */
DEFINE_SVC_UUID2(nxp_sip_svc_uid,
		 0x871de4ef, 0xedfc, 0x4209, 0xa4, 0x23,
		 0x8d, 0x23, 0x75, 0x9d, 0x3b, 0x9f);

#pragma weak nxp_plat_sip_handler
static uintptr_t nxp_plat_sip_handler(unsigned int smc_fid,
				      u_register_t x1,
				      u_register_t x2,
				      u_register_t x3,
				      u_register_t x4,
				      void *cookie,
				      void *handle,
				      u_register_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

uint64_t el2_2_aarch32(u_register_t smc_id, u_register_t start_addr,
		       u_register_t parm1, u_register_t parm2);

uint64_t prefetch_disable(u_register_t smc_id, u_register_t mask);
uint64_t bl31_get_porsr1(void);

static void clean_top_32b_of_param(uint32_t smc_fid,
				   u_register_t *px1,
				   u_register_t *px2,
				   u_register_t *px3,
				   u_register_t *px4)
{
	/* if parameters from SMC32. Clean top 32 bits */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		*px1 = *px1 & SMC32_PARAM_MASK;
		*px2 = *px2 & SMC32_PARAM_MASK;
		*px3 = *px3 & SMC32_PARAM_MASK;
		*px4 = *px4 & SMC32_PARAM_MASK;
	}
}

/* This function handles Layerscape defined SiP Calls */
static uintptr_t nxp_sip_handler(unsigned int smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie,
				 void *handle,
				 u_register_t flags)
{
	uint32_t ns;
	uint64_t ret;
	dram_regions_info_t *info_dram_regions;

	/* if parameter is sent from SMC32. Clean top 32 bits */
	clean_top_32b_of_param(smc_fid, &x1, &x2, &x3, &x4);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (ns == 0) {
		/* SiP SMC service secure world's call */
		;
	} else {
		/* SiP SMC service normal world's call */
		;
	}

	switch (smc_fid & SMC_FUNC_MASK) {
	case SIP_SVC_RNG:
		if (is_sec_enabled() == false) {
			NOTICE("SEC is disabled.\n");
			SMC_RET1(handle, SMC_UNK);
		}

		/* Return zero on failure */
		ret = get_random((int)x1);
		if (ret != 0) {
			SMC_RET2(handle, SMC_OK, ret);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
		/* break is not required as SMC_RETx return */
	case SIP_SVC_HUK:
		if (is_sec_enabled() == false) {
			NOTICE("SEC is disabled.\n");
			SMC_RET1(handle, SMC_UNK);
		}
		ret = get_hw_unq_key_blob_hw((uint8_t *) x1, (uint32_t) x2);

		if (ret == SMC_OK) {
			SMC_RET1(handle, SMC_OK);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
		/* break is not required as SMC_RETx return */
	case SIP_SVC_MEM_BANK:
		VERBOSE("Handling SMC SIP_SVC_MEM_BANK.\n");
		info_dram_regions = get_dram_regions_info();

		if (x1 == -1) {
			SMC_RET2(handle, SMC_OK,
					info_dram_regions->total_dram_size);
		} else if (x1 >= info_dram_regions->num_dram_regions) {
			SMC_RET1(handle, SMC_UNK);
		} else {
			SMC_RET3(handle, SMC_OK,
				info_dram_regions->region[x1].addr,
				info_dram_regions->region[x1].size);
		}
		/* break is not required as SMC_RETx return */
	case SIP_SVC_PREFETCH_DIS:
		VERBOSE("In SIP_SVC_PREFETCH_DIS call\n");
		ret = prefetch_disable(smc_fid, x1);
		if (ret == SMC_OK) {
			SMC_RET1(handle, SMC_OK);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
		/* break is not required as SMC_RETx return */
	case SIP_SVC_2_AARCH32:
		ret = el2_2_aarch32(smc_fid, x1, x2, x3);

		/* In success case, control should not reach here. */
		NOTICE("SMC: SIP_SVC_2_AARCH32 Failed.\n");
		SMC_RET1(handle, SMC_UNK);
		/* break is not required as SMC_RETx return */
	case SIP_SVC_PORSR1:
		ret = bl31_get_porsr1();
		SMC_RET2(handle, SMC_OK, ret);
		/* break is not required as SMC_RETx return */
	default:
		return nxp_plat_sip_handler(smc_fid, x1, x2, x3, x4,
				cookie, handle, flags);
	}
}

/* This function is responsible for handling all SiP calls */
static uintptr_t sip_smc_handler(unsigned int smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie,
				 void *handle,
				 u_register_t flags)
{
	switch (smc_fid & SMC_FUNC_MASK) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Layerscape SiP Service Calls. */
		SMC_RET1(handle, LS_COMMON_SIP_NUM_CALLS);
		break;
	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, nxp_sip_svc_uid);
		break;
	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, LS_SIP_SVC_VERSION_MAJOR,
			 LS_SIP_SVC_VERSION_MINOR);
		break;
	default:
		return nxp_sip_handler(smc_fid, x1, x2, x3, x4,
				       cookie, handle, flags);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	nxp_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);

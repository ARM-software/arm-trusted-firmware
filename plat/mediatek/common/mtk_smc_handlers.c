/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#if MTK_SIP_KERNEL_BOOT_ENABLE
#include <cold_boot.h>
#endif
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_sip_svc.h>

#define SMC_HANDLER_DEBUG(...) VERBOSE(__VA_ARGS__)
#define SMC_HANDLER_DEBUG_NOT_IMP_MSG "%s[0x%x] smc handler not implemented\n"
#define SMC_HANDLER_DEBUG_START_MSG "%s[0x%x] smc handler start, smc desc. index:%d\n"
#define SMC_HANDLER_DEBUG_END_MSG "%s[0x%x] smc handler end\n"

/*
 * These macros below are used to identify SIP calls from Kernel,
 * Hypervisor, or 2ndBootloader
 */
#define SIP_FID_ORI_MASK	(0xc000)
#define SIP_FID_ORI_SHIFT	(14)
#define SIP_FID_KERNEL		(0x0)
#define SIP_FID_KERNEL_VIA_GZ	(0x1)
#define SIP_FID_GZ		(0x2)

#define GET_SMC_ORI(_fid)	(((_fid) & SIP_FID_ORI_MASK) >> SIP_FID_ORI_SHIFT)
#define GET_SMC_ORI_NUM(_fid)	((_fid) & ~(SIP_FID_ORI_MASK))

#define is_from_nsel2(_ori)	(_ori == SIP_FID_GZ)
#define is_from_bl33(_ori) \
	((_ori != SIP_FID_GZ) && (is_el1_2nd_bootloader() == 1))
#define is_from_nsel1(_ori) \
	(((_ori == SIP_FID_KERNEL) || \
	 (_ori == SIP_FID_KERNEL_VIA_GZ)) && \
	 (is_el1_2nd_bootloader() == 0))

#define is_smc_forbidden(_ori) (_ori == SIP_FID_KERNEL_VIA_GZ)

#define MASK_32_BIT (0xffffffffU)
#define SMC_ID_EXPAND_AS_SMC_OPERATION(_smc_id, _smc_num) \
	case _smc_id##_AARCH32: \
	{ \
		x1 = x1 & MASK_32_BIT; \
		x2 = x2 & MASK_32_BIT; \
		x3 = x3 & MASK_32_BIT; \
		x4 = x4 & MASK_32_BIT; \
	} \
	/* fallthrough */ \
	case _smc_id##_AARCH64: \
	{ \
		if (_smc_id##_descriptor_index < 0) { \
			SMC_HANDLER_DEBUG(SMC_HANDLER_DEBUG_NOT_IMP_MSG, #_smc_id, smc_id); \
			break; \
		} \
		if (_smc_id##_descriptor_index >= smc_id_descriptor_max) { \
			SMC_HANDLER_DEBUG("smc descriptor index[%d] exceed max[%d]\n", \
					  _smc_id##_descriptor_index, smc_id_descriptor_max); \
			break; \
		} \
		SMC_HANDLER_DEBUG(SMC_HANDLER_DEBUG_START_MSG, #_smc_id, smc_id, \
				  _smc_id##_descriptor_index); \
		ret  = smc_handler_pool[_smc_id##_descriptor_index].smc_handler(x1,\
				x2, x3, x4, handle, &smc_ret); \
		SMC_HANDLER_DEBUG(SMC_HANDLER_DEBUG_END_MSG, #_smc_id, smc_id); \
		break; \
	}

#define SMC_ID_EXPAND_AS_DESCRIPTOR_INDEX(_smc_id, _smc_num) \
	short _smc_id##_descriptor_index __section("mtk_plat_ro") = -1;

MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_DESCRIPTOR_INDEX);
MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_DESCRIPTOR_INDEX);

IMPORT_SYM(uintptr_t, __MTK_SMC_POOL_START__, MTK_SMC_POOL_START);
IMPORT_SYM(uintptr_t, __MTK_SMC_POOL_END_UNALIGNED__, MTK_SMC_POOL_END_UNALIGNED);

static const struct smc_descriptor *smc_handler_pool;
static short smc_id_descriptor_max;

#if !MTK_SIP_KERNEL_BOOT_ENABLE
/*
 * If there is no SMC request needs to be served in 2nd bootloader,
 * disable the service path inherently.
 */
bool is_el1_2nd_bootloader(void)
{
	return false;
}
#endif

static void print_smc_descriptor(const struct smc_descriptor pool[])
{
	const struct smc_descriptor *p_smc_desc;

	INFO("print smc descriptor pool\n");
	for (p_smc_desc = &pool[0];
	     (char *)p_smc_desc < (char *)MTK_SMC_POOL_END_UNALIGNED;
	     p_smc_desc++) {
		INFO("descriptor name:%s\n", p_smc_desc->smc_name);
		INFO("descriptor index:%d\n", *p_smc_desc->smc_descriptor_index);
		INFO("smc id 32:0x%x, smc id 64:0x%x\n",
		     p_smc_desc->smc_id_aarch32, p_smc_desc->smc_id_aarch64);
	}
}

static int mtk_smc_handler_init(void)
{
	const struct smc_descriptor *iter;
	short index_cnt;
	int ret = 0;

	smc_handler_pool = (const struct smc_descriptor *)MTK_SMC_POOL_START;
	/* Designate descriptor index point to smc_handler_pool */
	for (index_cnt = 0, iter = &smc_handler_pool[0];
	     (char *)iter < (char *)MTK_SMC_POOL_END_UNALIGNED;
	     iter++, index_cnt++) {
		if (index_cnt < 0) {
			SMC_HANDLER_DEBUG("smc handler pool index overflow!\n");
			ret = -EPERM;
			assert(0);
			break;
		}
		*(iter->smc_descriptor_index) = index_cnt;
	}
	smc_id_descriptor_max = index_cnt;
	print_smc_descriptor(smc_handler_pool);
	return ret;
}
MTK_EARLY_PLAT_INIT(mtk_smc_handler_init);

/* This function handles Mediatek defined SiP Calls from Bootloader */
static uintptr_t mtk_smc_handler_bl33(uint32_t smc_id,
				      u_register_t x1,
				      u_register_t x2,
				      u_register_t x3,
				      u_register_t x4,
				      void *cookie,
				      void *handle,
				      u_register_t flags)
{
	uintptr_t ret = MTK_SIP_E_SUCCESS;
	struct smccc_res smc_ret = {0};

	switch (smc_id) {
		MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_SMC_OPERATION);
	default:
		INFO("BL33 SMC ID:0x%x not supported\n", smc_id);
		ret = SMC_UNK;
		break;
	}
	SMC_RET4(handle, ret, smc_ret.a1, smc_ret.a2, smc_ret.a3);
}

/* This function handles Mediatek defined SiP Calls from Kernel */
static uintptr_t mtk_smc_handler_nsel1(uint32_t smc_id,
				       u_register_t x1,
				       u_register_t x2,
				       u_register_t x3,
				       u_register_t x4,
				       void *cookie,
				       void *handle,
				       u_register_t flags)
{
	uintptr_t ret = MTK_SIP_E_SUCCESS;
	struct smccc_res smc_ret = {0};

	switch (smc_id) {
		MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_SMC_OPERATION);
	default:
		INFO("NSEL1 SMC ID:0x%x not supported\n", smc_id);
		ret = SMC_UNK;
		break;
	}
	SMC_RET4(handle, ret, smc_ret.a1, smc_ret.a2, smc_ret.a3);
}

static uintptr_t mtk_smc_handler(uint32_t smc_id,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie,
				 void *handle,
				 u_register_t flags)
{
	uintptr_t ret = SMC_UNK;
	uint32_t ns;
	uint32_t smc_ori;
	uint32_t smc_num;

	/* Get SMC Originator bit 14.15 */
	smc_ori = GET_SMC_ORI(smc_id);
	/* Get SMC Number. Clean bit 14.15 */
	smc_num = GET_SMC_ORI_NUM(smc_id);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	if (ns && is_smc_forbidden(smc_ori)) {
		ERROR("%s: Forbidden SMC call (0x%x)\n", __func__, smc_id);
		SMC_RET1(handle, ret);
	}

	if (!ns) {
		/* SiP SMC service secure world's call */
		INFO("Secure SMC ID:0x%x not supported\n", smc_id);
		SMC_RET1(handle, ret);
	}
	if (is_from_bl33(smc_ori)) {
		/* SiP SMC service secure bootloader's call */
		return mtk_smc_handler_bl33(smc_num, x1, x2, x3, x4,
					    cookie, handle, flags);
	} else if (is_from_nsel1(smc_ori)) {
		/* SiP SMC service kernel's call */
		return mtk_smc_handler_nsel1(smc_num, x1, x2, x3, x4,
					     cookie, handle, flags);
	}
	INFO("SMC ID:0x%x not supported\n", smc_id);
	SMC_RET1(handle, ret);
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	mtk_smc_handler,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	mtk_smc_handler
);

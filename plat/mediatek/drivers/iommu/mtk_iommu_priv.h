/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IOMMU_PRIV_H
#define IOMMU_PRIV_H

#include <common/debug.h>
#include <lib/mmio.h>
#include <mtk_iommu_plat.h>
#include <mtk_sip_svc.h>

#define LARB_CFG_ENTRY(bs, p_nr, dom)			\
	{ .base = (bs), .port_nr = (p_nr),		\
	  .dom_id = (dom), .to_sram = 0, }

#define LARB_CFG_ENTRY_WITH_PATH(bs, p_nr, dom, sram)	\
	{ .base = (bs), .port_nr = (p_nr),		\
	  .dom_id = (dom), .to_sram = (sram), }

#define IFR_MST_CFG_ENTRY(idx, bit)	\
	{ .cfg_addr_idx = (idx), .r_mmu_en_bit = (bit), }

#define SEC_IOMMU_CFG_ENTRY(s_bs)	\
	{ .base = (s_bs), }

enum IOMMU_ATF_CMD {
	IOMMU_ATF_CMD_CONFIG_SMI_LARB,		/* For mm master to enable iommu */
	IOMMU_ATF_CMD_CONFIG_INFRA_IOMMU,	/* For infra master to enable iommu */
	IOMMU_ATF_CMD_GET_SECURE_IOMMU_STATUS,	/* For secure iommu translation fault report */
	IOMMU_ATF_CMD_COUNT,
};

struct mtk_smi_larb_config {
	uint32_t base;
	uint32_t port_nr;
	uint32_t dom_id;
	uint32_t to_sram;
	uint32_t sec_en_msk;
};

struct mtk_ifr_mst_config {
	uint8_t cfg_addr_idx;
	uint8_t r_mmu_en_bit;
};

struct mtk_secure_iommu_config {
	uint32_t base;
};


#ifdef ATF_MTK_SMI_LARB_CFG_SUPPORT
/* mm smi larb security feature is used */
extern struct mtk_smi_larb_config *g_larb_cfg;
extern const unsigned int g_larb_num;
#endif

#ifdef ATF_MTK_INFRA_MASTER_CFG_SUPPORT
/* infra iommu is used */
extern struct mtk_ifr_mst_config *g_ifr_mst_cfg;
extern const unsigned int g_ifr_mst_num;
extern uint32_t *g_ifr_mst_cfg_base;
extern uint32_t *g_ifr_mst_cfg_offs;
extern void mtk_infra_iommu_enable_protect(void);
#endif

#ifdef ATF_MTK_IOMMU_CFG_SUPPORT
/* secure iommu is used */
extern struct mtk_secure_iommu_config *g_sec_iommu_cfg;
extern const unsigned int g_sec_iommu_num;
#endif

#endif	/* IOMMU_PRIV_H */

/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IOMMU_PRIV_H
#define IOMMU_PRIV_H

#include <common/debug.h>
#include <lib/mmio.h>
#include <mtk_sip_svc.h>

#define LARB_CFG_ENTRY(bs, p_nr, dom)			\
	{ .base = (bs), .port_nr = (p_nr),		\
	  .dom_id = (dom), .to_sram = 0, }

#define LARB_CFG_ENTRY_WITH_PATH(bs, p_nr, dom, sram)	\
	{ .base = (bs), .port_nr = (p_nr),		\
	  .dom_id = (dom), .to_sram = (sram), }

#define IFR_MST_CFG_ENTRY(idx, bit)	\
	{ .cfg_addr_idx = (idx), .r_mmu_en_bit = (bit), }

enum IOMMU_ATF_CMD {
	IOMMU_ATF_CMD_CONFIG_SMI_LARB,		/* For mm master to enable iommu */
	IOMMU_ATF_CMD_CONFIG_INFRA_IOMMU,	/* For infra master to enable iommu */
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

#endif	/* IOMMU_PRIV_H */

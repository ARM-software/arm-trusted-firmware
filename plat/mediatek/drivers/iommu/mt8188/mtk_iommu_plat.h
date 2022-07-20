/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IOMMU_PLAT_H
#define IOMMU_PLAT_H

#include <mtk_iommu_priv.h>

/* mm iommu */
#define SMI_LARB_NUM	(26)
extern struct mtk_smi_larb_config *g_larb_cfg;

/* infra iommu */
#define MMU_DEV_NUM	(1)
extern struct mtk_ifr_mst_config *g_ifr_mst_cfg;
extern uint32_t *g_ifr_mst_cfg_base;
extern uint32_t *g_ifr_mst_cfg_offs;

extern void mtk_infra_iommu_enable_protect(void);

#endif /* IOMMU_PLAT_H */

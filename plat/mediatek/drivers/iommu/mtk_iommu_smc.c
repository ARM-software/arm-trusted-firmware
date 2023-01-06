/*
 * Copyright (c) 2022-2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <mtk_iommu_plat.h>

/* defination */
/* smi larb */
#define SMI_LARB_NON_SEC_CON(port)	(0x380 + ((port) << 2))
#define PATH_SEL_MASK			(0xf0000) /* to sram (INT) */
#define SMI_LARB_SEC_CON_INT(port)	(0xf00 + ((port) << 2))
#define SMI_LARB_SEC_CON(port)		(0xf80 + ((port) << 2))
#define MMU_MASK			BIT(0)
#define MMU_EN(en)			((!!(en)) << 0)
#define SEC_MASK			BIT(1)
#define SEC_EN(en)			((!!(en)) << 1)
#define DOMAIN_MASK			(0x1f << 4)
#define SMI_MMU_EN(port)		(0x1 << (port))

/* infra master */
#define IFR_CFG_MMU_EN_MSK(r_bit)	(0x3 << (r_bit))

/* smi larb configure */
/*
 * If multimedia security config is enabled, the SMI config register must be
 * configurated in security world.
 * And the SRAM path is also configurated here to enhance security.
 */
static void mtk_smi_larb_port_config_to_sram(
				const struct mtk_smi_larb_config *larb,
				uint32_t port_id)
{
	mmio_clrbits_32(larb->base + SMI_LARB_SEC_CON_INT(port_id),
			MMU_MASK | SEC_MASK | DOMAIN_MASK);

	mmio_setbits_32(larb->base + SMI_LARB_NON_SEC_CON(port_id),
			PATH_SEL_MASK);
}

static void mtk_smi_port_config(const struct mtk_smi_larb_config *larb,
				uint32_t port_id, uint8_t mmu_en, uint8_t sec_en)
{
	mmio_clrsetbits_32(larb->base + SMI_LARB_SEC_CON(port_id),
			   MMU_MASK | SEC_MASK | DOMAIN_MASK,
			   MMU_EN(mmu_en) | SEC_EN(sec_en));
}

static int mtk_smi_larb_port_config_sec(uint32_t larb_id, uint32_t mmu_en_msk)
{
	uint32_t port_id, port_nr;
	const struct mtk_smi_larb_config *larb;
	uint32_t to_sram;
	uint8_t mmu_en;

	if (larb_id >= SMI_LARB_NUM) {
		return MTK_SIP_E_INVALID_PARAM;
	}

	larb = &g_larb_cfg[larb_id];
	port_nr = larb->port_nr;
	to_sram = larb->to_sram;

	for (port_id = 0; port_id < port_nr; port_id++) {
		if ((to_sram & BIT(port_id)) > 0U) {
			mtk_smi_larb_port_config_to_sram(larb, port_id);
			continue;
		}
		mmu_en = !!(mmu_en_msk & SMI_MMU_EN(port_id));
		mtk_smi_port_config(larb, port_id, mmu_en, 0);
	}

	return MTK_SIP_E_SUCCESS;
}

static int mtk_infra_master_config_sec(uint32_t dev_id_msk, uint32_t enable)
{
	const struct mtk_ifr_mst_config *ifr_cfg;
	uint32_t dev_id, reg_addr, reg_mask;

	mtk_infra_iommu_enable_protect();

	if (dev_id_msk >= BIT(MMU_DEV_NUM)) {
		return MTK_SIP_E_INVALID_PARAM;
	}

	for (dev_id = 0U; dev_id < MMU_DEV_NUM; dev_id++) {
		if ((dev_id_msk & BIT(dev_id)) == 0U) {
			continue;
		}

		ifr_cfg = &g_ifr_mst_cfg[dev_id];
		reg_addr = g_ifr_mst_cfg_base[(ifr_cfg->cfg_addr_idx)] +
			   g_ifr_mst_cfg_offs[(ifr_cfg->cfg_addr_idx)];
		reg_mask = IFR_CFG_MMU_EN_MSK(ifr_cfg->r_mmu_en_bit);

		if (enable > 0U) {
			mmio_setbits_32(reg_addr, reg_mask);
		} else {
			mmio_clrbits_32(reg_addr, reg_mask);
		}
	}

	return MTK_SIP_E_SUCCESS;
}

static u_register_t mtk_iommu_handler(u_register_t x1, u_register_t x2,
				      u_register_t x3, u_register_t x4,
				      void *handle, struct smccc_res *smccc_ret)
{
	uint32_t cmd_id = x1, mdl_id = x2, val = x3;
	int ret = MTK_SIP_E_NOT_SUPPORTED;

	(void)x4;
	(void)handle;

	switch (cmd_id) {
	case IOMMU_ATF_CMD_CONFIG_SMI_LARB:
		ret = mtk_smi_larb_port_config_sec(mdl_id, val);
		break;
	case IOMMU_ATF_CMD_CONFIG_INFRA_IOMMU:
		ret = mtk_infra_master_config_sec(mdl_id, val);
		break;
	default:
		break;
	}

	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_IOMMU_CONTROL, mtk_iommu_handler);

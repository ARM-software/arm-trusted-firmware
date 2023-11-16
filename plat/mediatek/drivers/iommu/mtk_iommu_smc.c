/*
 * Copyright (c) 2022-2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <mtk_iommu_priv.h>

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

/* secure iommu */
#define MMU_INT_CONTROL0		(0x120)
#define INT_CLR				BIT(12)
#define MMU_FAULT_ST1			(0x134)
#define MMU_AXI_0_ERR_MASK		GENMASK(6, 0)
#define MMU_AXI_FAULT_STATUS(bus)	(0x13c + (bus) * 8)
#define MMU_AXI_INVLD_PA(bus)		(0x140 + (bus) * 8)
#define MMU_AXI_INT_ID(bus)		(0x150 + (bus) * 4)

/* smi larb configure */
/*
 * If multimedia security config is enabled, the SMI config register must be
 * configurated in security world.
 * And the SRAM path is also configurated here to enhance security.
 */
#ifdef ATF_MTK_SMI_LARB_CFG_SUPPORT

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

	if (larb_id >= g_larb_num) {
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

#endif /* ATF_MTK_SMI_LARB_CFG_SUPPORT */

/* infra iommu configure */
#ifdef ATF_MTK_INFRA_MASTER_CFG_SUPPORT

static int mtk_infra_master_config_sec(uint32_t dev_id_msk, uint32_t enable)
{
	const struct mtk_ifr_mst_config *ifr_cfg;
	uint32_t dev_id, reg_addr, reg_mask;

	mtk_infra_iommu_enable_protect();

	if (dev_id_msk >= BIT(g_ifr_mst_num)) {
		return MTK_SIP_E_INVALID_PARAM;
	}

	for (dev_id = 0U; dev_id < g_ifr_mst_num; dev_id++) {
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
#endif /* ATF_MTK_INFRA_MASTER_CFG_SUPPORT */

/* secure iommu */
#ifdef ATF_MTK_IOMMU_CFG_SUPPORT
/* Report secure IOMMU fault status to normal world for the debug version */
static int mtk_secure_iommu_fault_report(uint32_t sec_mmu_base,
					 uint32_t *f_sta, uint32_t *f_pa,
					 uint32_t *f_id)
{
	const struct mtk_secure_iommu_config *mmu_cfg = NULL;
	uint32_t __maybe_unused bus_id, fault_type;
	uint32_t i;
	int ret = MTK_SIP_E_NOT_SUPPORTED;

	for (i = 0; i < g_sec_iommu_num; i++) {
		if (g_sec_iommu_cfg[i].base == sec_mmu_base) {
			mmu_cfg = &g_sec_iommu_cfg[i];
			break;
		}
	}

	if (!mmu_cfg)
		return MTK_SIP_E_INVALID_PARAM;
#if DEBUG
	fault_type = mmio_read_32(mmu_cfg->base + MMU_FAULT_ST1);
	bus_id = (fault_type & MMU_AXI_0_ERR_MASK) ? 0 : 1;

	if (f_sta)
		*f_sta = mmio_read_32(mmu_cfg->base + MMU_AXI_FAULT_STATUS(bus_id));
	if (f_pa)
		*f_pa = mmio_read_32(mmu_cfg->base + MMU_AXI_INVLD_PA(bus_id));
	if (f_id)
		*f_id = mmio_read_32(mmu_cfg->base + MMU_AXI_INT_ID(bus_id));
	ret = MTK_SIP_E_SUCCESS;
#endif
	mmio_setbits_32(mmu_cfg->base + MMU_INT_CONTROL0, INT_CLR);

	return ret;
}
#endif /* ATF_MTK_IOMMU_CFG_SUPPORT */

u_register_t mtk_iommu_handler(u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4,
			void *handle, struct smccc_res *smccc_ret)
{
	uint32_t cmd_id = x1, mdl_id = x2, val = x3;
	int ret = MTK_SIP_E_NOT_SUPPORTED;

	(void)x4;
	(void)handle;

	switch (cmd_id) {
#ifdef ATF_MTK_SMI_LARB_CFG_SUPPORT
	case IOMMU_ATF_CMD_CONFIG_SMI_LARB:
		ret = mtk_smi_larb_port_config_sec(mdl_id, val);
		break;
#endif
#ifdef ATF_MTK_INFRA_MASTER_CFG_SUPPORT
	case IOMMU_ATF_CMD_CONFIG_INFRA_IOMMU:
		ret = mtk_infra_master_config_sec(mdl_id, val);
		break;
#endif
#ifdef ATF_MTK_IOMMU_CFG_SUPPORT
	case IOMMU_ATF_CMD_GET_SECURE_IOMMU_STATUS:
		(void)val;
		ret = mtk_secure_iommu_fault_report(mdl_id,
					(uint32_t *)&smccc_ret->a1,
					(uint32_t *)&smccc_ret->a2,
					(uint32_t *)&smccc_ret->a3);
		break;
#endif
	default:
		break;
	}

	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_IOMMU_CONTROL, mtk_iommu_handler);

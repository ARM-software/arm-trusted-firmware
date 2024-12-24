/*
 * Copyright (c) 2023-2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

/* TF-A system header */
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_rv.h"
#include "apusys_rv_mbox_mpu.h"
#include "apusys_rv_pwr_ctrl.h"
#include "apusys_rv_sec_info.h"
#ifdef CONFIG_MTK_APUSYS_SEC_CTRL
#include "apusys_security_ctrl_perm.h"
#endif
#include "apusys_security_ctrl_plat.h"
#include <drivers/apusys_rv_public.h>
#include <mtk_mmap_pool.h>
#include <mtk_sip_svc.h>

#ifdef CONFIG_MTK_APUSYS_RV_APUMMU_SUPPORT
#include "apusys_ammu.h"
#endif

static spinlock_t apusys_rv_lock;

void apusys_rv_mbox_mpu_init(void)
{
	int i;

	for (i = 0; i < APU_MBOX_NUM; i++) {
		mmio_write_32(APU_MBOX_FUNC_CFG(i),
			      (MBOX_CTRL_LOCK |
			       (mbox_mpu_setting_tab[i].no_mpu << MBOX_NO_MPU_SHIFT)));
		mmio_write_32(APU_MBOX_DOMAIN_CFG(i),
			      (MBOX_CTRL_LOCK |
			       (mbox_mpu_setting_tab[i].rx_ns << MBOX_RX_NS_SHIFT) |
			       (mbox_mpu_setting_tab[i].rx_domain << MBOX_RX_DOMAIN_SHIFT) |
			       (mbox_mpu_setting_tab[i].tx_ns << MBOX_TX_NS_SHIFT) |
			       (mbox_mpu_setting_tab[i].tx_domain << MBOX_TX_DOMAIN_SHIFT)));
	}
}

int apusys_kernel_apusys_rv_setup_reviser(void)
{
	spin_lock(&apusys_rv_lock);

	mmio_write_32(USERFW_CTXT, CFG_4GB_SEL_EN | CFG_4GB_SEL);
	mmio_write_32(SECUREFW_CTXT, CFG_4GB_SEL_EN | CFG_4GB_SEL);

	mmio_write_32(UP_IOMMU_CTRL, MMU_CTRL_LOCK | MMU_CTRL | MMU_EN);

	mmio_write_32(UP_NORMAL_DOMAIN_NS,
		      (UP_NORMAL_DOMAIN << UP_DOMAIN_SHIFT) | (UP_NORMAL_NS << UP_NS_SHIFT));
	mmio_write_32(UP_PRI_DOMAIN_NS,
		      (UP_PRI_DOMAIN << UP_DOMAIN_SHIFT) | (UP_PRI_NS << UP_NS_SHIFT));

	mmio_write_32(UP_CORE0_VABASE0,
		      VLD | PARTIAL_ENABLE | (THREAD_NUM_PRI << THREAD_NUM_SHIFT));
	mmio_write_32(UP_CORE0_MVABASE0, VASIZE_1MB | (APU_SEC_FW_IOVA >> MVA_34BIT_SHIFT));

	mmio_write_32(UP_CORE0_VABASE1,
		      VLD | PARTIAL_ENABLE | (THREAD_NUM_NORMAL << THREAD_NUM_SHIFT));
	mmio_write_32(UP_CORE0_MVABASE1, VASIZE_1MB | (APU_SEC_FW_IOVA >> MVA_34BIT_SHIFT));

	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_reset_mp(void)
{
	spin_lock(&apusys_rv_lock);

	mmio_write_32(MD32_SYS_CTRL, MD32_SYS_CTRL_RST);

	dsb();
	udelay(RESET_DEALY_US);

	mmio_write_32(MD32_SYS_CTRL, MD32_G2B_CG_EN | MD32_DBG_EN | MD32_DM_AWUSER_IOMMU_EN |
		      MD32_DM_ARUSER_IOMMU_EN | MD32_PM_AWUSER_IOMMU_EN | MD32_PM_ARUSER_IOMMU_EN |
		      MD32_SOFT_RSTN);

	mmio_write_32(MD32_CLK_CTRL, MD32_CLK_EN);
	mmio_write_32(UP_WAKE_HOST_MASK0, WDT_IRQ_EN);
	mmio_write_32(UP_WAKE_HOST_MASK1, MBOX0_IRQ_EN | MBOX1_IRQ_EN | MBOX2_IRQ_EN);

	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_setup_boot(void)
{
	spin_lock(&apusys_rv_lock);

	mmio_write_32(MD32_BOOT_CTRL, APU_SEC_FW_IOVA);

	mmio_write_32(MD32_PRE_DEFINE, (PREDEFINE_CACHE_TCM << PREDEF_1G_OFS) |
		      (PREDEFINE_CACHE << PREDEF_2G_OFS) | (PREDEFINE_CACHE << PREDEF_3G_OFS) |
		      (PREDEFINE_CACHE << PREDEF_4G_OFS));

	apusys_infra_dcm_setup();

	spin_unlock(&apusys_rv_lock);
	return 0;
}

int apusys_kernel_apusys_rv_start_mp(void)
{
	spin_lock(&apusys_rv_lock);
	mmio_write_32(MD32_RUNSTALL, MD32_RUN);
	spin_unlock(&apusys_rv_lock);

	return 0;
}

static int hw_sema2_release(uint32_t timeout)
{
#ifdef CONFIG_MTK_APUSYS_RV_COREDUMP_WA_SUPPORT
	int ret;

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_USER, 0, timeout, 0);
	if (ret) {
		ERROR("%s: HW semaphore release timeout\n", __func__);
	}

	return ret;
#else
	return 0;
#endif
}

static int hw_sema2_acquire(uint32_t timeout)
{
#ifdef CONFIG_MTK_APUSYS_RV_COREDUMP_WA_SUPPORT
	int ret;

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_USER, 1, timeout, 0);
	if (ret) {
		ERROR("%s: HW semaphore acquire timeout\n", __func__);
	}

	return ret;
#else
	return 0;
#endif
}

int apusys_kernel_apusys_rv_stop_mp(void)
{
	int ret;

	ret = hw_sema2_acquire(HW_SEM_TIMEOUT);
	if (ret)
		return ret;

	spin_lock(&apusys_rv_lock);
	mmio_write_32(MD32_RUNSTALL, MD32_STALL);
	spin_unlock(&apusys_rv_lock);

	ret = hw_sema2_release(HW_SEM_TIMEOUT);

	return ret;
}

int apusys_kernel_apusys_rv_setup_sec_mem(void)
{
	int ret = 0;

	spin_lock(&apusys_rv_lock);

	ret = apusys_plat_setup_sec_mem();
	if (ret != 0)
		ERROR(MODULE_TAG "%s: set emimpu protection failed\n", __func__);

	spin_unlock(&apusys_rv_lock);
	return ret;
}

int apusys_kernel_apusys_rv_disable_wdt_isr(void)
{
	int ret;

	ret = hw_sema2_acquire(0);
	if (ret)
		return ret;

	spin_lock(&apusys_rv_lock);
	mmio_clrbits_32(WDT_CTRL0, WDT_EN);
	spin_unlock(&apusys_rv_lock);

	ret = hw_sema2_release(0);

	return ret;
}

int apusys_kernel_apusys_rv_clear_wdt_isr(void)
{
	int ret;

	ret = hw_sema2_acquire(HW_SEM_TIMEOUT);
	if (ret)
		return ret;

	spin_lock(&apusys_rv_lock);
	mmio_clrbits_32(UP_INT_EN2, DBG_APB_EN);
	mmio_write_32(WDT_INT, WDT_INT_W1C);
	spin_unlock(&apusys_rv_lock);

	ret = hw_sema2_release(HW_SEM_TIMEOUT);

	return ret;
}

int apusys_kernel_apusys_rv_cg_gating(void)
{
	int ret;

	ret = hw_sema2_acquire(HW_SEM_TIMEOUT);
	if (ret)
		return ret;

	spin_lock(&apusys_rv_lock);
	mmio_write_32(MD32_CLK_CTRL, MD32_CLK_DIS);
	spin_unlock(&apusys_rv_lock);

	ret = hw_sema2_release(HW_SEM_TIMEOUT);

	return ret;
}

int apusys_kernel_apusys_rv_cg_ungating(void)
{
	int ret;

	ret = hw_sema2_acquire(HW_SEM_TIMEOUT);
	if (ret)
		return ret;

	spin_lock(&apusys_rv_lock);
	mmio_write_32(MD32_CLK_CTRL, MD32_CLK_EN);
	spin_unlock(&apusys_rv_lock);

	ret = hw_sema2_release(HW_SEM_TIMEOUT);

	return ret;
}

int apusys_kernel_apusys_rv_setup_apummu(void)
{
	spin_lock(&apusys_rv_lock);

#ifdef CONFIG_MTK_APUSYS_SEC_CTRL
	sec_set_rv_dns();
#endif

#ifdef CONFIG_MTK_APUSYS_RV_APUMMU_SUPPORT
	uint32_t apummu_tcm_sz_select = 0;

	if (APU_MD32_TCM_SZ <= 0x20000)
		apummu_tcm_sz_select = APUMMU_PAGE_LEN_128KB;
	else if (APU_MD32_TCM_SZ <= 0x40000)
		apummu_tcm_sz_select = APUMMU_PAGE_LEN_256KB;
	else if (APU_MD32_TCM_SZ <= 0x80000)
		apummu_tcm_sz_select = APUMMU_PAGE_LEN_512KB;
	else if (APU_MD32_TCM_SZ <= 0x100000)
		apummu_tcm_sz_select = APUMMU_PAGE_LEN_1MB;
	else {
		ERROR("%s: APU_MD32_TCM_SZ = 0x%x > 1MB", __func__, APU_MD32_TCM_SZ);
		spin_unlock(&apusys_rv_lock);
		return -EINVAL;
	}

	INFO("%s: apummu_tcm_sz_select = %u\n", __func__, apummu_tcm_sz_select);
	rv_boot(APU_SEC_FW_IOVA, 0, APUMMU_PAGE_LEN_1MB,
		APU_MD32_TCM, apummu_tcm_sz_select);
#endif

	spin_unlock(&apusys_rv_lock);
	return 0;
}

int apusys_kernel_apusys_rv_pwr_ctrl(enum APU_PWR_OP op)
{
	return apusys_rv_pwr_ctrl(op);
}

#ifdef CONFIG_MTK_APUSYS_LOGTOP_SUPPORT
int apusys_kernel_apusys_logtop_reg_dump(uint32_t op, struct smccc_res *smccc_ret)
{
	int ret = 0;
	uint8_t smc_op;
	uint32_t reg_addr[MAX_SMC_OP_NUM];
	uint32_t i;

	if (op == 0) {
		ERROR("%s empty op = 0x%08x\n", MODULE_TAG, op);
		return -EINVAL;
	}

	for (i = 0; i < MAX_SMC_OP_NUM; i++) {
		smc_op = (op >> (LOGTOP_OP_SHIFT * i)) & LOGTOP_OP_MASK;
		switch (smc_op) {
		case SMC_OP_APU_LOG_BUF_NULL:
			reg_addr[i] = 0x0;
			break;
		case SMC_OP_APU_LOG_BUF_T_SIZE:
			reg_addr[i] = APU_LOG_BUF_T_SIZE;
			break;
		case SMC_OP_APU_LOG_BUF_W_PTR:
			reg_addr[i] = APU_LOG_BUF_W_PTR;
			break;
		case SMC_OP_APU_LOG_BUF_R_PTR:
			reg_addr[i] = APU_LOG_BUF_R_PTR;
			break;
		case SMC_OP_APU_LOG_BUF_CON:
			reg_addr[i] = APU_LOGTOP_CON;
			break;
		default:
			ERROR("%s unknown op = 0x%08x\n", MODULE_TAG, smc_op);
			return -EINVAL;
		}
	}

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_LOGGER_USER, 1, 0, 0);
	if (ret)
		return ret;

	for (i = 0; i < MAX_SMC_OP_NUM; i++) {
		if (reg_addr[i] == 0)
			continue;

		switch (i) {
		case 0:
			smccc_ret->a1 = mmio_read_32(reg_addr[i]);
			break;
		case 1:
			smccc_ret->a2 = mmio_read_32(reg_addr[i]);
			break;
		case 2:
			smccc_ret->a3 = mmio_read_32(reg_addr[i]);
			break;
		}
	}

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_LOGGER_USER, 0, 0, 0);
	if (ret)
		ERROR("%s(%d): sem release timeout\n", __func__, op);

	return ret;
}

static int apusys_kernel_apusys_logtop_reg_rw(uint32_t op, uint32_t write_val,
					      bool w1c, struct smccc_res *smccc_ret)
{
	int ret = 0;
	uint32_t reg_addr = 0, reg_val = 0;

	switch (op) {
	case SMC_OP_APU_LOG_BUF_R_PTR:
		reg_addr = APU_LOG_BUF_R_PTR;
		break;
	case SMC_OP_APU_LOG_BUF_CON:
		reg_addr = APU_LOGTOP_CON;
		break;
	default:
		ERROR("%s unknown or not support op = %x\n", MODULE_TAG, op);
		return -EINVAL;
	}

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_LOGGER_USER, 1, 0, 0);
	if (ret)
		return ret;

	if (w1c) {
		reg_val = mmio_read_32(reg_addr);
		mmio_write_32(reg_addr, reg_val);
		smccc_ret->a1 = reg_val;
	} else {
		mmio_write_32(reg_addr, write_val);
	}

	ret = apu_hw_sema_ctl(HW_SEMA2, HW_SEMA_LOGGER_USER, 0, 0, 0);
	if (ret)
		ERROR("%s(%d): sem release timeout\n", __func__, op);

	return ret;
}

int apusys_kernel_apusys_logtop_reg_write(uint32_t op, uint32_t write_val,
					  struct smccc_res *smccc_ret)
{
	return apusys_kernel_apusys_logtop_reg_rw(op, write_val, false, smccc_ret);
}

int apusys_kernel_apusys_logtop_reg_w1c(uint32_t op, struct smccc_res *smccc_ret)
{
	return apusys_kernel_apusys_logtop_reg_rw(op, 0, true, smccc_ret);
}

#endif /* CONFIG_MTK_APUSYS_LOGTOP_SUPPORT */

int apusys_rv_cold_boot_clr_mbox_dummy(void)
{
#ifdef SUPPORT_APU_CLEAR_MBOX_DUMMY
	mmio_write_32(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_DUMMY, 0);
#else
	WARN("Not support clear mbox dummy on this platform\n");
#endif
	return 0;
}

#ifdef CONFIG_MTK_APUSYS_RV_IOMMU_HW_SEM_SUPPORT
int apusys_rv_iommu_hw_sem_trylock(void)
{
	return rv_iommu_hw_sem_trylock();
}

int apusys_rv_iommu_hw_sem_unlock(void)
{
	return rv_iommu_hw_sem_unlock();
}
#endif /* CONFIG_MTK_APUSYS_RV_IOMMU_HW_SEM_SUPPORT */

int apusys_rv_setup_ce_bin(void)
{
#ifdef CONFIG_MTK_APUSYS_CE_SUPPORT
	uintptr_t apusys_rv_sec_buf_pa;
	struct apusys_secure_info_t *apusys_secure_info;
	struct ce_main_hdr_t *ce_main_hdr;
	struct ce_sub_hdr_t *ce_sub_hdr;
	unsigned int cnt, i, reg_val;
	uint64_t ce_sub_hdr_bin;
	int ret;

	apusys_rv_sec_buf_pa = APU_RESERVE_MEMORY;
	/* create mapping */
	ret = mmap_add_dynamic_region(apusys_rv_sec_buf_pa, apusys_rv_sec_buf_pa,
				      round_up(APU_RESERVE_SIZE, PAGE_SIZE),
				      MT_MEMORY | MT_RW | MT_NS);
	if (ret) {
		ERROR("%s: mmap_add_dynamic_region() fail, ret=0x%x\n", __func__, ret);
		return ret;
	}

	apusys_secure_info = (struct apusys_secure_info_t *)
				(apusys_rv_sec_buf_pa + APU_SEC_INFO_OFFSET);

	ce_main_hdr = (struct ce_main_hdr_t *)(apusys_rv_sec_buf_pa +
		apusys_secure_info->ce_bin_ofs);
	ce_sub_hdr = (struct ce_sub_hdr_t *)((uintptr_t)ce_main_hdr + ce_main_hdr->hdr_size);

	if (ce_main_hdr->magic != CE_MAIN_MAGIC) {
		ERROR("%s: invalid header\n", __func__);
		return -EINVAL;
	}

	cnt = 0;

	while (ce_sub_hdr->magic == CE_SUB_MAGIC && cnt < ce_main_hdr->bin_count) {
		VERBOSE("%s: job (%d), magic (0x%x)\n", __func__,
			ce_sub_hdr->ce_enum, ce_sub_hdr->magic);

		ce_sub_hdr_bin = (uint64_t)ce_sub_hdr + ce_sub_hdr->bin_offset;

		for (i = 0; i < ce_sub_hdr->bin_size; i += sizeof(uint32_t)) {
			reg_val = *(uint32_t *)(ce_sub_hdr_bin + i);
			mmio_write_32(ce_sub_hdr->mem_st + i, reg_val);
		}

		if (ce_sub_hdr->hw_entry) {
			mmio_clrsetbits_32(ce_sub_hdr->hw_entry,
					   ce_sub_hdr->hw_entry_mask << ce_sub_hdr->hw_entry_bit,
					   (ce_sub_hdr->hw_entry_val & ce_sub_hdr->hw_entry_mask)
					   << ce_sub_hdr->hw_entry_bit);
		}

		ce_sub_hdr = (struct ce_sub_hdr_t *)(ce_sub_hdr_bin + ce_sub_hdr->bin_size);
		cnt++;
	}

	mmap_remove_dynamic_region(apusys_rv_sec_buf_pa,
				   round_up(APU_RESERVE_SIZE, PAGE_SIZE));

	INFO("%s: setup CE binary done\n", __func__);
#else
	WARN("Not support CE on this platform\n");
#endif
	return 0;
}

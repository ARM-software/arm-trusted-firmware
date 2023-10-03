/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_rv.h"
#include "apusys_rv_mbox_mpu.h"
#include "emi_mpu.h"

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
	static bool apusys_rv_setup_reviser_called;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_setup_reviser_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_setup_reviser_called = true;

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
	static bool apusys_rv_reset_mp_called;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_reset_mp_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_reset_mp_called = true;

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
	static bool apusys_rv_setup_boot_called;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_setup_boot_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_setup_boot_called = true;

	mmio_write_32(MD32_BOOT_CTRL, APU_SEC_FW_IOVA);

	mmio_write_32(MD32_PRE_DEFINE, (PREDEFINE_CACHE_TCM << PREDEF_1G_OFS) |
		      (PREDEFINE_CACHE << PREDEF_2G_OFS) | (PREDEFINE_CACHE << PREDEF_3G_OFS) |
		      (PREDEFINE_CACHE << PREDEF_4G_OFS));

	spin_unlock(&apusys_rv_lock);
	return 0;
}

int apusys_kernel_apusys_rv_start_mp(void)
{
	static bool apusys_rv_start_mp_called;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_start_mp_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_start_mp_called = true;

	mmio_write_32(MD32_RUNSTALL, MD32_RUN);

	spin_unlock(&apusys_rv_lock);

	return 0;
}

static bool watch_dog_is_timeout(void)
{
	if (mmio_read_32(WDT_INT) != WDT_INT_W1C) {
		ERROR(MODULE_TAG "%s: WDT does not timeout\n", __func__);
		return false;
	}
	return true;
}

int apusys_kernel_apusys_rv_stop_mp(void)
{
	static bool apusys_rv_stop_mp_called;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_stop_mp_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	if (watch_dog_is_timeout() == false) {
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_stop_mp_called = true;

	mmio_write_32(MD32_RUNSTALL, MD32_STALL);

	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_setup_sec_mem(void)
{
	static bool apusys_rv_setup_sec_mem_called;
	int ret;

	spin_lock(&apusys_rv_lock);

	if (apusys_rv_setup_sec_mem_called) {
		WARN(MODULE_TAG "%s: already initialized\n", __func__);
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	apusys_rv_setup_sec_mem_called = true;

	ret = set_apu_emi_mpu_region();
	if (ret != 0) {
		ERROR(MODULE_TAG "%s: set emimpu protection failed\n", __func__);
	}

	spin_unlock(&apusys_rv_lock);
	return ret;
}

int apusys_kernel_apusys_rv_disable_wdt_isr(void)
{
	spin_lock(&apusys_rv_lock);
	mmio_clrbits_32(WDT_CTRL0, WDT_EN);
	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_clear_wdt_isr(void)
{
	spin_lock(&apusys_rv_lock);
	mmio_clrbits_32(UP_INT_EN2, DBG_APB_EN);
	mmio_write_32(WDT_INT, WDT_INT_W1C);
	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_cg_gating(void)
{
	spin_lock(&apusys_rv_lock);

	if (watch_dog_is_timeout() == false) {
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	mmio_write_32(MD32_CLK_CTRL, MD32_CLK_DIS);
	spin_unlock(&apusys_rv_lock);

	return 0;
}

int apusys_kernel_apusys_rv_cg_ungating(void)
{
	spin_lock(&apusys_rv_lock);

	if (watch_dog_is_timeout() == false) {
		spin_unlock(&apusys_rv_lock);
		return -1;
	}

	mmio_write_32(MD32_CLK_CTRL, MD32_CLK_EN);
	spin_unlock(&apusys_rv_lock);

	return 0;
}

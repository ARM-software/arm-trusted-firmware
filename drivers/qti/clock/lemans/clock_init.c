/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for lemans (qcs9075): image init hooks and the
 * GPU GDSC and boot-IMEM teardown sequences.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include "clock_hwio.h"
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

/* Bounded retry count for GDSC power-up/down completion polling. */
#define CLOCK_GDSC_POLL_RETRIES	500U

#define CLOCK_GX_RESET_SETTLE_DELAY_US	150U
#define CLOCK_STEP_DELAY_US		1U
#define CLOCK_LPASS_RESTART_DELAY_US	500U

static int clock_enable_gpu_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL,
			   GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_BMSK,
			   2U << GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL2,
			   GPU_CX_MISC_GFX_SPARE_CNTL2_BITS_9_1_BMSK,
			   250U << GPU_CX_MISC_GFX_SPARE_CNTL2_BITS_9_1_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_CFG2_GDSCR,
			   GPUCC_GPU_CC_GX_CFG2_GDSCR_DLY_ASSERT_CLAMP_MEM_BMSK,
			   7U << GPUCC_GPU_CC_GX_CFG2_GDSCR_DLY_ASSERT_CLAMP_MEM_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_BCR,
			   GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_BCR,
			   GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_MISC,
			   GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(CLOCK_GX_RESET_SETTLE_DELAY_US);
	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_BCR,
			   GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_BCR,
			   GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_MISC,
			   GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(CLOCK_STEP_DELAY_US);

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_GDSCR,
			   GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_GX_CFG_GDSCR) &
		 GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_BMSK) >>
		GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(CLOCK_STEP_DELAY_US);
	}

	if (retry == 0U) {
		WARN("Clock: GPU GX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_gpu_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_GDSCR,
			   GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);

	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_MISC,
			   GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(CLOCK_GX_RESET_SETTLE_DELAY_US);

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);

	while ((((mmio_read_32(GPUCC_GPU_CC_GX_CFG_GDSCR) &
		 GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_BMSK) >>
		GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(CLOCK_STEP_DELAY_US);
	}

	if (retry == 0U) {
		WARN("Clock: GPU GX GDSC power-down timed out\n");
	}
}

static int clock_enable_gpu_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_PWR_DWN_BMSK,
			   1U << GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_PWR_DWN_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_STS_BMSK,
			   1U << GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_STS_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_NRET_BMSK,
			   1U << GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_NRET_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_RET_BMSK,
			   1U << GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_RET_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_BMSK,
			   0U << GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_SHFT);
	mmio_clrsetbits_32(GPU_CX_MISC_GFX_SPARE_CNTL0,
			   GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_ACK_BMSK,
			   0U << GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_ACK_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_CFG2_GDSCR,
			   GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_BMSK,
			   1U << GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_SHFT);

	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GDSCR,
			   GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_CX_CFG_GDSCR) &
		 GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_BMSK) >>
		GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(CLOCK_STEP_DELAY_US);
	}

	if (retry == 0U) {
		WARN("Clock: GPU CX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_gpu_cx_gdsc(void)
{
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GDSCR,
			   GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

static int clock_enable_gpu_smmu_clk(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_setbits_32(GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
			GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK_CLK_ENABLE_BMSK);
	while (((mmio_read_32(GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK) &
		 GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK_CLK_OFF_BMSK) != 0U) &&
	       (--retry > 0U)) {
		udelay(CLOCK_STEP_DELAY_US);
	}

	if (retry == 0U) {
		ERROR("Clock: GPU SMMU clock stays off\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_boot_imem(void)
{
	uint32_t delay;

	mmio_setbits_32(GCC_CFG_NOC_LPASS_CBCR, HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK);
	for (delay = 500U;
	     delay != 0U && (mmio_read_32(GCC_CFG_NOC_LPASS_CBCR) &
			     HAL_CLK_BRANCH_CTRL_REG_CLK_OFF_FMSK) != 0U;
	     delay--)
		udelay(CLOCK_STEP_DELAY_US);

	mmio_setbits_32(LPASS_LPASS_TOP_CC_LPI_Q6_AXIM_HS_CBCR,
			HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK);
	for (delay = 500U;
	     delay != 0U && (mmio_read_32(LPASS_LPASS_TOP_CC_LPI_Q6_AXIM_HS_CBCR) &
			     HAL_CLK_BRANCH_CTRL_REG_CLK_OFF_FMSK) != 0U;
	     delay--)
		udelay(CLOCK_STEP_DELAY_US);

	/*
	 * Clean and invalidate the dcache before disabling BOOT_IMEM, to
	 * ensure there are no stale entries.
	 */
	dcsw_op_all(DCCISW);
	udelay(100U);

	/*
	 * BOOT_IMEM is no longer required by TF-A. This is a sticky bit: once 1
	 * is written, only the tcsr_fp_alt_ares reset can return it to 0.
	 */
	mmio_clrsetbits_32(TCSR_BOOT_IMEM_DISABLE,
			   TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK,
			   1U << TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_SHFT);

	/*
	 * Restore the Q6 RCG XO input to XO and disable the AXIS2 clock, which
	 * is no longer required as IMEM.
	 */
	mmio_clrsetbits_32(GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR,
			   GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_BMSK,
			   0U << GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_SHFT);
	mmio_clrbits_32(GCC_LPASS_AXIS2_CBCR, HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK);

	mmio_write_32(LPASS_LPASS_ALT_RESET_Q6SS, 1U);
	udelay(50U);

	/*
	 * Trigger the SS func_ares signal. Skipping this can result in rogue
	 * transactions from the subsystem under certain conditions (e.g. when
	 * LPI_CX collapses before LPASS PIL has executed).
	 */
	mmio_clrsetbits_32(AOSS_CC_LPASS_RESTART,
			   AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK,
			   1U << AOSS_CC_LPASS_RESTART_SS_RESTART_SHFT);
	udelay(CLOCK_LPASS_RESTART_DELAY_US);
	mmio_clrsetbits_32(AOSS_CC_LPASS_RESTART,
			   AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK,
			   0U << AOSS_CC_LPASS_RESTART_SS_RESTART_SHFT);
	udelay(CLOCK_LPASS_RESTART_DELAY_US);

	mmio_write_32(LPASS_LPASS_ALT_RESET_Q6SS, 0U);
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_source *gpll0 = &drv_ctxt->cfg->sources[CLOCK_SOURCE_GPLL0];
	int ret;

	/*
	 * Disabling IMEM must occur before enabling the init groups, since it
	 * may involve resets that undo some of the clocks enabled there.
	 */
	clock_disable_boot_imem();

	/* Enable clocks required for init. */
	ret = clock_group_enable(CLOCK_GROUP_INIT);
	if (ret != 0) {
		return ret;
	}
	ret = clock_group_enable(CLOCK_GROUP_QDSS);
	if (ret != 0) {
		return ret;
	}

	/*
	 * These two clocks are needed before turning on the CX/GX GDSCs: the
	 * memory sleep staggering logic requires them, and the GDSCs are
	 * required for GPU SMMU access.
	 */
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GMU_CBCR,
			   GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK,
			   GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK_CLK_ENABLE_BMSK,
			   1U << GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK_CLK_ENABLE_SHFT);

	if (clock_enable_gpu_cx_gdsc() != 0) {
		ERROR("Clock: GPU CX GDSC enable failed\n");
		return -ETIMEDOUT;
	}
	if (clock_enable_gpu_gx_gdsc() != 0) {
		ERROR("Clock: GPU GX GDSC enable failed\n");
		return -ETIMEDOUT;
	}

	/*
	 * To ensure TF-A can always vote for the GDSCs, disallow SW-override
	 * mode, since overriding also disables the voting logic.
	 */
	mmio_clrsetbits_32(MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
			   0U << MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_CAM_CC_GDSCR,
			   GCC_CAM_CC_GDSCR_SW_OVERRIDE_BMSK,
			   0U << GCC_CAM_CC_GDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_LPASS_TBU_SGDSCR,
			   GCC_APCS_TZ_LPASS_TBU_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_LPASS_TBU_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_USB30_PRIM_SGDSCR,
			   GCC_APCS_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_MMNOC_SGDSCR,
			   GCC_APCS_TZ_MMNOC_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_MMNOC_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_PHY_SGDSCR,
			   GCC_APCS_TZ_UFS_PHY_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_UFS_PHY_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_IPA_SGDSCR,
			   GCC_APCS_TZ_IPA_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_IPA_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_MMU3_SGDSCR,
			   GCC_APCS_TZ_MMU3_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_MMU3_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_TURING_0_SGDSCR,
			   GCC_APCS_TZ_TURING_0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_TURING_0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_TURING_1_SGDSCR,
			   GCC_APCS_TZ_TURING_1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_TURING_1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_PCIE_0_SGDSCR,
			   GCC_APCS_TZ_PCIE_0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_PCIE_0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_PCIE_1_SGDSCR,
			   GCC_APCS_TZ_PCIE_1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_PCIE_1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_EMAC1_SGDSCR,
			   GCC_APCS_TZ_EMAC1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_EMAC1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_EMAC0_SGDSCR,
			   GCC_APCS_TZ_EMAC0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_EMAC0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_CARD_SGDSCR,
			   GCC_APCS_TZ_UFS_CARD_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_UFS_CARD_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_USB30_SEC_SGDSCR,
			   GCC_APCS_TZ_USB30_SEC_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << GCC_APCS_TZ_USB30_SEC_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE,
		LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
		0U);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE,
		LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
		0U);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE,
		LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
		0U);

	mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
			   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
			   0U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
			   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
			   0U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);

	/*
	 * Enable GDSC retention mode, required for SMMU to retain state across
	 * power collapse.
	 */
	mmio_clrsetbits_32(MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
			   1U << MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_CAM_CC_GDSCR,
			   GCC_CAM_CC_GDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_CAM_CC_GDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_LPASS_TBU_SGDSCR,
			   GCC_APCS_TZ_LPASS_TBU_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_LPASS_TBU_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_USB30_PRIM_SGDSCR,
			   GCC_APCS_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_MMNOC_SGDSCR,
			   GCC_APCS_TZ_MMNOC_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_MMNOC_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_PHY_SGDSCR,
			   GCC_APCS_TZ_UFS_PHY_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_UFS_PHY_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_IPA_SGDSCR,
			   GCC_APCS_TZ_IPA_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_IPA_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_MMU3_SGDSCR,
			   GCC_APCS_TZ_MMU3_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_MMU3_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_TURING_0_SGDSCR,
			   GCC_APCS_TZ_TURING_0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_TURING_0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_EMAC1_SGDSCR,
			   GCC_APCS_TZ_EMAC1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_EMAC1_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_EMAC0_SGDSCR,
			   GCC_APCS_TZ_EMAC0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_EMAC0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_CARD_SGDSCR,
			   GCC_APCS_TZ_UFS_CARD_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_UFS_CARD_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_USB30_SEC_SGDSCR,
			   GCC_APCS_TZ_USB30_SEC_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_USB30_SEC_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_PCIE_0_SGDSCR,
			   GCC_APCS_TZ_PCIE_0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_PCIE_0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_APCS_TZ_PCIE_1_SGDSCR,
			   GCC_APCS_TZ_PCIE_1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_PCIE_1_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE,
			   LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
		1U << LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE,
			   LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
		1U << LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);
	mmio_clrsetbits_32(LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE,
			   LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
		1U << LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);

	mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
			   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
			   1U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
			   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
			   1U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);

	mmio_clrsetbits_32(GCC_APCS_TZ_TURING_1_SGDSCR,
			   GCC_APCS_TZ_TURING_1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_APCS_TZ_TURING_1_SGDSCR_RETAIN_FF_ENABLE_SHFT);

	/*
	 * Crashes were observed during CX power collapse without this. Since
	 * DPM (always sourcing from GPLL0) handshakes with the CX ARC during
	 * CXPC, GPLL0 must be kept on.
	 */
	ret = clock_source_enable(gpll0);
	if (ret != 0) {
		return ret;
	}

	/*
	 * SWA for GPU BIMC error / stage-2 faults: MMU TLB corruption due to
	 * the SMMU SRAM clock turning on before power-on.
	 */
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GMU_CBCR,
			   GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
			   1U << GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);

	/* Accesses to the GPU SMMU registers hang without its clock. */
	ret = clock_enable_gpu_smmu_clk();
	if (ret != 0) {
		return ret;
	}

	/*
	 * iris_cc_gcc_clks_off does not go high,
	 * blocking MMCX collapse, because VIDEO_CC_MVS1_CMD_RCGR__ROOT_OFF is
	 * not asserted and the SREG PSCBC controller is stuck.
	 */
	if (chipinfo_get_chip_version() >= CHIPINFO_VERSION(2, 0)) {
		mmio_clrsetbits_32(VCODEC_VIDEO_CC_MVS1_CBCR,
				   VCODEC_VIDEO_CC_MVS1_CBCR_SW_ONLY_EN_BMSK,
				   1U << VCODEC_VIDEO_CC_MVS1_CBCR_SW_ONLY_EN_SHFT);
		mmio_clrsetbits_32(VCODEC_VIDEO_CC_MVS1_CBCR,
				   VCODEC_VIDEO_CC_MVS1_CBCR_CLK_ENABLE_BMSK,
				   0U << VCODEC_VIDEO_CC_MVS1_CBCR_CLK_ENABLE_SHFT);
		mmio_clrsetbits_32(VCODEC_VIDEO_CC_MVS1_CBCR,
				   VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
				   1U << VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_PERIPH_ON_SHFT);
		mmio_clrsetbits_32(VCODEC_VIDEO_CC_MVS1_CBCR,
				   VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_CORE_ON_BMSK,
				   1U << VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_CORE_ON_SHFT);
	}

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/* Restore clocks enabled during init to their original state. */
	mmio_clrbits_32(GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
			GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK_CLK_ENABLE_BMSK);
	clock_disable_gpu_gx_gdsc();
	clock_disable_gpu_cx_gdsc();
	clock_group_disable(CLOCK_GROUP_QDSS);
	clock_group_disable(CLOCK_GROUP_INIT);

	/* Re-enable the QDSS STM clock; needed because of a DS-exit issue. */
	mmio_setbits_32(GCC_QDSS_STM_CBCR, HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK);

	/* Program the *PLL_SECURE registers back to secure mode. */
	mmio_write_32(CL0_GOLD_PLL_SECURE, 0x0U);
	mmio_write_32(CL1_GOLD_PLL_SECURE, 0x0U);
	mmio_write_32(CL0_L3_PLL_SECURE, 0x0U);
	mmio_write_32(CL1_L3_PLL_SECURE, 0x0U);

	return 0;
}

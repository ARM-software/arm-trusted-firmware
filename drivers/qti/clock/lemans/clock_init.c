/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for lemans (qcs9075): image init hooks.
 */

#include <stdint.h>

#include <arch_helpers.h>
#include "clock_hwio.h"
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

static const uintptr_t lemans_lpass_votes[] = {
	LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE,
	LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE,
	LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE,
};

/* Secure GDSC controls not tied to a part chipinfo reports. */
static const uintptr_t lemans_tz_gdscs[] = {
	GCC_APCS_TZ_USB30_PRIM_SGDSCR,
	GCC_APCS_TZ_MMNOC_SGDSCR,
	GCC_APCS_TZ_UFS_PHY_SGDSCR,
	GCC_APCS_TZ_IPA_SGDSCR,
	GCC_APCS_TZ_MMU3_SGDSCR,
	GCC_APCS_TZ_EMAC1_SGDSCR,
	GCC_APCS_TZ_EMAC0_SGDSCR,
	GCC_APCS_TZ_UFS_CARD_SGDSCR,
	GCC_APCS_TZ_USB30_SEC_SGDSCR,
};

static void clock_disable_boot_imem(void)
{
	hoya_clock_lpass_imem_on();

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
	mmio_setbits_32(TCSR_BOOT_IMEM_DISABLE,
			TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK);

	hoya_clock_lpass_q6_reset();
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_source *gpll0 = &drv_ctxt->cfg->sources[CLOCK_SOURCE_GPLL0];
	unsigned int i;
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
	 * The init group does not clock the GPU, display, LPASS and video
	 * clock controllers of a part chipinfo reports disabled, and a
	 * disabled part has no SMMU state to retain.
	 */
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		ret = hoya_clock_gpu_init();
		if (ret != 0) {
			return ret;
		}
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_DISPLAY, 0U)) {
		hoya_clock_gdsc_retain(MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL);
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_CAMERA, 0U)) {
		hoya_clock_gdsc_retain(GCC_CAM_CC_GDSCR);
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_AUDIO, 0U)) {
		hoya_clock_tz_gdsc_init(GCC_APCS_TZ_LPASS_TBU_SGDSCR);
		hoya_clock_retention_votes(lemans_lpass_votes,
					   ARRAY_SIZE(lemans_lpass_votes));
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_NSP, 0U)) {
		hoya_clock_tz_gdsc_init(GCC_APCS_TZ_TURING_0_SGDSCR);
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_NSP, 1U)) {
		hoya_clock_tz_gdsc_init(GCC_APCS_TZ_TURING_1_SGDSCR);
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_PCIE, 0U)) {
		hoya_clock_tz_gdsc_init(GCC_APCS_TZ_PCIE_0_SGDSCR);
	}
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_PCIE, 1U)) {
		hoya_clock_tz_gdsc_init(GCC_APCS_TZ_PCIE_1_SGDSCR);
	}
	for (i = 0U; i < ARRAY_SIZE(lemans_tz_gdscs); i++) {
		hoya_clock_tz_gdsc_init(lemans_tz_gdscs[i]);
	}

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
	 * iris_cc_gcc_clks_off does not go high,
	 * blocking MMCX collapse, because VIDEO_CC_MVS1_CMD_RCGR__ROOT_OFF is
	 * not asserted and the SREG PSCBC controller is stuck.
	 */
	if ((chipinfo_get_chip_version() >= CHIPINFO_VERSION(2, 0)) &&
	    !chipinfo_is_part_disabled(CHIPINFO_PART_VIDEO, 0U)) {
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
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		hoya_clock_gpu_gdsc_disable();
	}
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

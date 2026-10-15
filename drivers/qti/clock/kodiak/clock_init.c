/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for kodiak (qcs6490): the secure UFS GDSC
 * retention vote, and GPU CX for the GPU SMMU setup.
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_APCS_TZ_UFS_PHY_SGDSCR	0x00153008U
#define SGDSCR_RETAIN_FF_ENABLE		BIT_32(1)
#define SGDSCR_SW_OVERRIDE		BIT_32(0)

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define GPU_CC_CX_GMU_CBCR		0x03d91098U
#define GPU_CC_HUB_CX_INT_CBCR		0x03d91204U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	0x03d93000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	0x03d93004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX	0x03d94004U

#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_CLK_OFF			BIT_32(31)
#define TZ_VOTE_GDS_SW_COLLAPSE		BIT_32(0)
#define TZ_VOTE_GDS_PWR_ON		BIT_32(31)
#define TZ_GDSC_CTRL_RETAIN_FF_ENABLE	BIT_32(11)

/* GPU CX takes about 1.7 ms to power up. */
#define GPU_CX_TIMEOUT_US		10000U

/*
 * Branches the GPU SMMU registers need: the GPU config interface, then,
 * with CX on, the GPU SMMU clock vote and the CX GMU and hub clocks.
 */
static const uintptr_t gpu_cx_cbcrs[] = {
	GCC_GPU_CFG_AHB_CBCR,
	GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
	GPU_CC_CX_GMU_CBCR,
	GPU_CC_HUB_CX_INT_CBCR,
};

/* What clock_init_image() turned on, for clock_post_init_image() to undo. */
static bool gpu_cx_voted;
static bool gpu_cbcr_enabled[ARRAY_SIZE(gpu_cx_cbcrs)];

static bool wait_bits(uintptr_t addr, uint32_t mask, uint32_t want)
{
	uint64_t timeout = timeout_init_us(GPU_CX_TIMEOUT_US);

	while ((mmio_read_32(addr) & mask) != want) {
		if (timeout_elapsed(timeout)) {
			return (mmio_read_32(addr) & mask) == want;
		}
	}

	return true;
}

static int gpu_branch_on(size_t i)
{
	if ((mmio_read_32(gpu_cx_cbcrs[i]) & CBCR_CLK_ENABLE) == 0U) {
		mmio_setbits_32(gpu_cx_cbcrs[i], CBCR_CLK_ENABLE);
		gpu_cbcr_enabled[i] = true;
	}
	if (!wait_bits(gpu_cx_cbcrs[i], CBCR_CLK_OFF, 0U)) {
		ERROR("Clock: GPU branch 0x%lx stays off\n", gpu_cx_cbcrs[i]);
		return -ETIMEDOUT;
	}

	return 0;
}

/*
 * XBL leaves GPU CX collapsed with its clocks off. Turn CX and the branches
 * its SMMU needs on for qti_smmu_clocked_init(), with register retention, so
 * that the registers it writes survive the CX collapses that follow.
 */
static int gpu_cx_on(void)
{
	int ret = gpu_branch_on(0U);

	if (ret != 0) {
		return ret;
	}

	if ((mmio_read_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS) &
	     TZ_VOTE_GDS_SW_COLLAPSE) != 0U) {
		mmio_clrbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
				TZ_VOTE_GDS_SW_COLLAPSE);
		gpu_cx_voted = true;
	}
	if (!wait_bits(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_PWR_ON,
		       TZ_VOTE_GDS_PWR_ON)) {
		ERROR("Clock: GPU CX stays off\n");
		return -ETIMEDOUT;
	}

	for (size_t i = 1U; (ret == 0) && (i < ARRAY_SIZE(gpu_cx_cbcrs)); i++) {
		ret = gpu_branch_on(i);
	}
	mmio_setbits_32(GPU_CC_TZ_GDSC_CTRL_GPU_CX,
			TZ_GDSC_CTRL_RETAIN_FF_ENABLE);

	return ret;
}

/* Give back every vote and branch that gpu_cx_on() turned on. */
static void gpu_cx_restore(void)
{
	for (size_t i = ARRAY_SIZE(gpu_cx_cbcrs) - 1U; i > 0U; i--) {
		if (gpu_cbcr_enabled[i]) {
			mmio_clrbits_32(gpu_cx_cbcrs[i], CBCR_CLK_ENABLE);
		}
	}
	if (gpu_cx_voted) {
		mmio_setbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
				TZ_VOTE_GDS_SW_COLLAPSE);
	}
	if (gpu_cbcr_enabled[0]) {
		mmio_clrbits_32(gpu_cx_cbcrs[0], CBCR_CLK_ENABLE);
	}
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/*
	 * The OS collapses the UFS GDSC and resumes the link from hibern8,
	 * which needs the host state retained. SW_OVERRIDE stays clear, as
	 * in QTISECLIB.
	 */
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_PHY_SGDSCR, SGDSCR_SW_OVERRIDE,
			   SGDSCR_RETAIN_FF_ENABLE);

	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return 0;
	}

	return gpu_cx_on();
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	gpu_cx_restore();

	return 0;
}

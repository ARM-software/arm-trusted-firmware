/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * GPU CX/GX GDSC power sequences of the hoya SoCs whose GPU_CC has them,
 * built against the chipset's clock_hwio.h.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include "clock_hwio.h"
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>

#define CLOCK_GX_RESET_SETTLE_DELAY_US	150U
#define CLOCK_STEP_DELAY_US		1U

static struct clock_power_domain_desc gpu_cx_gdsc = {
	.gdscr_addr = GPUCC_GPU_CC_CX_GDSCR,
};

static struct clock_power_domain_desc gpu_gx_gdsc = {
	.gdscr_addr = GPUCC_GPU_CC_GX_GDSCR,
};

static struct clock_desc gpu_cx_gmu_clk = {
	.cbcr_addr = GPUCC_GPU_CC_CX_GMU_CBCR,
};

/* The TZ vote registers have the CBCR CLK_ENABLE and CLK_OFF fields. */
static struct clock_desc gpu_hub_cx_int_clk = {
	.cbcr_addr = GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK,
};

static struct clock_desc gpu_smmu_clk = {
	.cbcr_addr = GPUCC_GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
};

static int clock_enable_gpu_gx_gdsc(void)
{
	mmio_clrsetbits_32(GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL,
			   GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_BMSK,
			   2U << GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_SHFT);
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

	clock_hal_enable_power_domain(&gpu_gx_gdsc);
	if (clock_hal_wait_for_power_domain_on(&gpu_gx_gdsc) != 0) {
		WARN("Clock: GPU GX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_gpu_gx_gdsc(void)
{
	clock_hal_disable_power_domain(&gpu_gx_gdsc);

	mmio_clrsetbits_32(GPUCC_GPU_CC_ACD_MISC,
			   GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(CLOCK_GX_RESET_SETTLE_DELAY_US);

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);

	if (clock_hal_wait_for_power_domain_off(&gpu_gx_gdsc) != 0) {
		WARN("Clock: GPU GX GDSC power-down timed out\n");
	}
}

static int clock_enable_gpu_cx_gdsc(void)
{
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_CFG2_GDSCR,
			   GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_BMSK,
			   1U << GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_SHFT);

	clock_hal_enable_power_domain(&gpu_cx_gdsc);
	if (clock_hal_wait_for_power_domain_on(&gpu_cx_gdsc) != 0) {
		WARN("Clock: GPU CX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

/* Power the GPU CX and GX domains for the boot-time GPU SMMU setup. */
int hoya_clock_gpu_init(void)
{
	/*
	 * These two clocks are needed before turning on the CX/GX GDSCs: the
	 * memory sleep staggering logic requires them, and the GDSCs are
	 * required for GPU SMMU access.
	 */
	clock_hal_set_clock(&gpu_cx_gmu_clk, true);
	clock_hal_set_clock(&gpu_hub_cx_int_clk, true);

	if (clock_enable_gpu_cx_gdsc() != 0) {
		ERROR("Clock: GPU CX GDSC enable failed\n");
		return -ETIMEDOUT;
	}
	if (clock_enable_gpu_gx_gdsc() != 0) {
		ERROR("Clock: GPU GX GDSC enable failed\n");
		return -ETIMEDOUT;
	}

	/*
	 * Keep the GDSC votes working and retain the GPU CX state, which holds
	 * the GPU SMMU registers, across CX collapse.
	 */
	hoya_clock_gdsc_retain(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX);
	hoya_clock_gdsc_retain(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX);

	/*
	 * SWA for GPU BIMC error / stage-2 faults: MMU TLB corruption due to
	 * the SMMU SRAM clock turning on before power-on.
	 */
	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GMU_CBCR,
			   GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
			   1U << GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);

	/* Accesses to the GPU SMMU registers hang without its clock. */
	clock_hal_set_clock(&gpu_smmu_clk, true);
	if (clock_hal_wait_for_clock_on(&gpu_smmu_clk) != 0) {
		ERROR("Clock: GPU SMMU clock stays off\n");
		return -ETIMEDOUT;
	}

	return 0;
}

void hoya_clock_gpu_gdsc_disable(void)
{
	clock_hal_set_clock(&gpu_smmu_clk, false);
	clock_disable_gpu_gx_gdsc();
	clock_hal_disable_power_domain(&gpu_cx_gdsc);
}

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <gpcdma.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>
#include <stdbool.h>
#include <tegra_def.h>

/* DMA channel registers */
#define DMA_CH_CSR				U(0x0)
#define DMA_CH_CSR_WEIGHT_SHIFT			U(10)
#define DMA_CH_CSR_XFER_MODE_SHIFT		U(21)
#define DMA_CH_CSR_DMA_MODE_MEM2MEM		U(4)
#define DMA_CH_CSR_DMA_MODE_FIXEDPATTERN	U(6)
#define DMA_CH_CSR_IRQ_MASK_ENABLE		(U(1) << 15)
#define DMA_CH_CSR_RUN_ONCE			(U(1) << 27)
#define DMA_CH_CSR_ENABLE			(U(1) << 31)

#define DMA_CH_STAT				U(0x4)
#define DMA_CH_STAT_BUSY			(U(1) << 31)

#define DMA_CH_SRC_PTR				U(0xC)

#define DMA_CH_DST_PTR				U(0x10)

#define DMA_CH_HI_ADR_PTR			U(0x14)
#define DMA_CH_HI_ADR_PTR_SRC_MASK		U(0xFF)
#define DMA_CH_HI_ADR_PTR_DST_SHIFT		U(16)
#define DMA_CH_HI_ADR_PTR_DST_MASK		U(0xFF)

#define DMA_CH_MC_SEQ				U(0x18)
#define DMA_CH_MC_SEQ_REQ_CNT_SHIFT		U(25)
#define DMA_CH_MC_SEQ_REQ_CNT_VAL		U(0x10)
#define DMA_CH_MC_SEQ_BURST_SHIFT		U(23)
#define DMA_CH_MC_SEQ_BURST_16_WORDS		U(0x3)

#define DMA_CH_WORD_COUNT			U(0x20)
#define DMA_CH_FIXED_PATTERN			U(0x34)
#define DMA_CH_TZ				U(0x38)
#define DMA_CH_TZ_ACCESS_ENABLE			U(0)
#define DMA_CH_TZ_ACCESS_DISABLE		U(3)

#define MAX_TRANSFER_SIZE			(1U*1024U*1024U*1024U)	/* 1GB */
#define GPCDMA_TIMEOUT_MS			U(100)
#define GPCDMA_RESET_BIT			(U(1) << 1)

static bool init_done;

static void tegra_gpcdma_write32(uint32_t offset, uint32_t val)
{
	mmio_write_32(TEGRA_GPCDMA_BASE + offset, val);
}

static uint32_t tegra_gpcdma_read32(uint32_t offset)
{
	return mmio_read_32(TEGRA_GPCDMA_BASE + offset);
}

static void tegra_gpcdma_init(void)
{
	/* assert reset for DMA engine */
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_GPCDMA_RST_SET_REG_OFFSET,
		      GPCDMA_RESET_BIT);

	udelay(2);

	/* de-assert reset for DMA engine */
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_GPCDMA_RST_CLR_REG_OFFSET,
		      GPCDMA_RESET_BIT);
}

static void tegra_gpcdma_memcpy_priv(uint64_t dst_addr, uint64_t src_addr,
				     uint32_t num_bytes, uint32_t mode)
{
	uint32_t val, timeout = 0;
	int32_t ret = 0;

	/* sanity check byte count */
	if ((num_bytes > MAX_TRANSFER_SIZE) || ((num_bytes & 0x3U) != U(0))) {
		ret = -EINVAL;
	}

	/* initialise GPCDMA block */
	if (!init_done) {
		tegra_gpcdma_init();
		init_done = true;
	}

	/* make sure channel isn't busy */
	val = tegra_gpcdma_read32(DMA_CH_STAT);
	if ((val & DMA_CH_STAT_BUSY) == DMA_CH_STAT_BUSY) {
		ERROR("DMA channel is busy\n");
		ret = -EBUSY;
	}

	if (ret == 0) {

		/* disable any DMA transfers */
		tegra_gpcdma_write32(DMA_CH_CSR, 0);

		/* enable DMA access to TZDRAM */
		tegra_gpcdma_write32(DMA_CH_TZ, DMA_CH_TZ_ACCESS_ENABLE);

		/* configure MC sequencer */
		val = (DMA_CH_MC_SEQ_REQ_CNT_VAL << DMA_CH_MC_SEQ_REQ_CNT_SHIFT) |
		      (DMA_CH_MC_SEQ_BURST_16_WORDS << DMA_CH_MC_SEQ_BURST_SHIFT);
		tegra_gpcdma_write32(DMA_CH_MC_SEQ, val);

		/* reset fixed pattern */
		tegra_gpcdma_write32(DMA_CH_FIXED_PATTERN, 0);

		/* populate src and dst address registers */
		tegra_gpcdma_write32(DMA_CH_SRC_PTR, (uint32_t)src_addr);
		tegra_gpcdma_write32(DMA_CH_DST_PTR, (uint32_t)dst_addr);

		val = (uint32_t)((src_addr >> 32) & DMA_CH_HI_ADR_PTR_SRC_MASK);
		val |= (uint32_t)(((dst_addr >> 32) & DMA_CH_HI_ADR_PTR_DST_MASK) <<
			DMA_CH_HI_ADR_PTR_DST_SHIFT);
		tegra_gpcdma_write32(DMA_CH_HI_ADR_PTR, val);

		/* transfer size (in words) */
		tegra_gpcdma_write32(DMA_CH_WORD_COUNT, ((num_bytes >> 2) - 1U));

		/* populate value for CSR */
		val = (mode << DMA_CH_CSR_XFER_MODE_SHIFT) |
		      DMA_CH_CSR_RUN_ONCE | (U(1) << DMA_CH_CSR_WEIGHT_SHIFT) |
		      DMA_CH_CSR_IRQ_MASK_ENABLE;
		tegra_gpcdma_write32(DMA_CH_CSR, val);

		/* enable transfer */
		val = tegra_gpcdma_read32(DMA_CH_CSR);
		val |= DMA_CH_CSR_ENABLE;
		tegra_gpcdma_write32(DMA_CH_CSR, val);

		/* wait till transfer completes */
		do {

			/* read the status */
			val = tegra_gpcdma_read32(DMA_CH_STAT);
			if ((val & DMA_CH_STAT_BUSY) != DMA_CH_STAT_BUSY) {
				break;
			}

			mdelay(1);
			timeout++;

		} while (timeout < GPCDMA_TIMEOUT_MS);

		/* flag timeout error */
		if (timeout == GPCDMA_TIMEOUT_MS) {
			ERROR("DMA transfer timed out\n");
		}

		dsbsy();

		/* disable DMA access to TZDRAM */
		tegra_gpcdma_write32(DMA_CH_TZ, DMA_CH_TZ_ACCESS_DISABLE);
		isb();
	}
}

/*******************************************************************************
 * Memcpy using GPCDMA block (Mem2Mem copy)
 ******************************************************************************/
void tegra_gpcdma_memcpy(uint64_t dst_addr, uint64_t src_addr,
			 uint32_t num_bytes)
{
	tegra_gpcdma_memcpy_priv(dst_addr, src_addr, num_bytes,
				 DMA_CH_CSR_DMA_MODE_MEM2MEM);
}

/*******************************************************************************
 * Memset using GPCDMA block (Fixed pattern write)
 ******************************************************************************/
void tegra_gpcdma_zeromem(uint64_t dst_addr, uint32_t num_bytes)
{
	tegra_gpcdma_memcpy_priv(dst_addr, 0, num_bytes,
				 DMA_CH_CSR_DMA_MODE_FIXEDPATTERN);
}

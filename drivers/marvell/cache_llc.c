/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* LLC driver is the Last Level Cache (L3C) driver
 * for Marvell SoCs in AP806, AP807, and AP810
 */

#include <assert.h>

#include <arch_helpers.h>
#include <drivers/marvell/cache_llc.h>
#include <drivers/marvell/ccu.h>
#include <lib/mmio.h>

#include <mvebu_def.h>

#define CCU_HTC_CR(ap_index)		(MVEBU_CCU_BASE(ap_index) + 0x200)
#define CCU_SET_POC_OFFSET		5

extern void ca72_l2_enable_unique_clean(void);

void llc_cache_sync(int ap_index)
{
	mmio_write_32(LLC_SYNC(ap_index), 0);
	/* Atomic write, no need to wait */
}

void llc_flush_all(int ap_index)
{
	mmio_write_32(L2X0_CLEAN_INV_WAY(ap_index), LLC_WAY_MASK);
	llc_cache_sync(ap_index);
}

void llc_clean_all(int ap_index)
{
	mmio_write_32(L2X0_CLEAN_WAY(ap_index), LLC_WAY_MASK);
	llc_cache_sync(ap_index);
}

void llc_inv_all(int ap_index)
{
	mmio_write_32(L2X0_INV_WAY(ap_index), LLC_WAY_MASK);
	llc_cache_sync(ap_index);
}

void llc_disable(int ap_index)
{
	llc_flush_all(ap_index);
	mmio_write_32(LLC_CTRL(ap_index), 0);
	dsbishst();
}

void llc_enable(int ap_index, int excl_mode)
{
	uint32_t val;

	dsbsy();
	llc_inv_all(ap_index);
	dsbsy();

	val = LLC_CTRL_EN;
	if (excl_mode)
		val |= LLC_EXCLUSIVE_EN;

	mmio_write_32(LLC_CTRL(ap_index), val);
	dsbsy();
}

int llc_is_exclusive(int ap_index)
{
	uint32_t reg;

	reg = mmio_read_32(LLC_CTRL(ap_index));

	if ((reg & (LLC_CTRL_EN | LLC_EXCLUSIVE_EN)) ==
		   (LLC_CTRL_EN | LLC_EXCLUSIVE_EN))
		return 1;

	return 0;
}

void llc_runtime_enable(int ap_index)
{
	uint32_t reg;

	reg = mmio_read_32(LLC_CTRL(ap_index));
	if (reg & LLC_CTRL_EN)
		return;

	INFO("Enabling LLC\n");

	/*
	 * Enable L2 UniqueClean evictions with data
	 *  Note: this configuration assumes that LLC is configured
	 *	  in exclusive mode.
	 *	  Later on in the code this assumption will be validated
	 */
	ca72_l2_enable_unique_clean();
	llc_enable(ap_index, 1);

	/* Set point of coherency to DDR.
	 * This is required by units which have SW cache coherency
	 */
	reg = mmio_read_32(CCU_HTC_CR(ap_index));
	reg |= (0x1 << CCU_SET_POC_OFFSET);
	mmio_write_32(CCU_HTC_CR(ap_index), reg);
}

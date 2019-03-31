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
	mmio_write_32(LLC_CLEAN_INV_WAY(ap_index), LLC_ALL_WAYS_MASK);
	llc_cache_sync(ap_index);
}

void llc_clean_all(int ap_index)
{
	mmio_write_32(LLC_CLEAN_WAY(ap_index), LLC_ALL_WAYS_MASK);
	llc_cache_sync(ap_index);
}

void llc_inv_all(int ap_index)
{
	mmio_write_32(LLC_INV_WAY(ap_index), LLC_ALL_WAYS_MASK);
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

#if LLC_SRAM
void llc_sram_enable(int ap_index)
{
	uint32_t tc, way;
	uint32_t way_addr;

	/* Lockdown all available ways for all traffic classes */
	for (tc = 0; tc < LLC_TC_NUM; tc++)
		mmio_write_32(LLC_TCN_LOCK(ap_index, tc), LLC_WAY_MASK);

	/* Clear the high bits of SRAM address */
	mmio_write_32(LLC_BANKED_MNT_AHR(ap_index), 0);

	way_addr = PLAT_MARVELL_TRUSTED_RAM_BASE;
	for (way = 0; way < LLC_WAYS; way++) {
		/* Trigger allocation block command */
		mmio_write_32(LLC_BLK_ALOC(ap_index),
			      LLC_BLK_ALOC_BASE_ADDR(way_addr) |
			      LLC_BLK_ALOC_WAY_DATA_CLR |
			      LLC_BLK_ALOC_WAY_ID(way));
		way_addr += LLC_WAY_SIZE;
	}
	llc_enable(ap_index, 1);
}

void llc_sram_disable(int ap_index)
{
	uint32_t tc;

	/* Disable the line lockings */
	for (tc = 0; tc < LLC_TC_NUM; tc++)
		mmio_write_32(LLC_TCN_LOCK(ap_index, tc), 0);

	/* Invalidate all ways */
	llc_inv_all(ap_index);
}
#endif /* LLC_SRAM */

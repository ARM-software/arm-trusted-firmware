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
int llc_sram_enable(int ap_index, int size)
{
	uint32_t tc, way, ways_to_allocate;
	uint32_t way_addr;

	if ((size <= 0) || (size > LLC_SIZE) || (size % LLC_WAY_SIZE))
		return -1;

	llc_enable(ap_index, 1);
	llc_inv_all(ap_index);

	ways_to_allocate = size / LLC_WAY_SIZE;

	/* Lockdown all available ways for all traffic classes */
	for (tc = 0; tc < LLC_TC_NUM; tc++)
		mmio_write_32(LLC_TCN_LOCK(ap_index, tc), LLC_ALL_WAYS_MASK);

	/* Clear the high bits of SRAM address */
	mmio_write_32(LLC_BANKED_MNT_AHR(ap_index), 0);

	way_addr = PLAT_MARVELL_TRUSTED_RAM_BASE;
	for (way = 0; way < ways_to_allocate; way++) {
		/* Trigger allocation block command */
		mmio_write_32(LLC_BLK_ALOC(ap_index),
			      LLC_BLK_ALOC_BASE_ADDR(way_addr) |
			      LLC_BLK_ALOC_WAY_DATA_SET |
			      LLC_BLK_ALOC_WAY_ID(way));
		way_addr += LLC_WAY_SIZE;
	}
	return 0;
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

int llc_sram_test(int ap_index, int size, char *msg)
{
	uintptr_t addr, end_addr;
	uint32_t data = 0;

	if ((size <= 0) || (size > LLC_SIZE))
		return -1;

	INFO("=== LLC SRAM WRITE test %s\n", msg);
	for (addr = PLAT_MARVELL_TRUSTED_RAM_BASE,
	     end_addr = PLAT_MARVELL_TRUSTED_RAM_BASE + size;
	     addr < end_addr; addr += 4) {
		mmio_write_32(addr, addr);
	}
	INFO("=== LLC SRAM WRITE test %s PASSED\n", msg);
	INFO("=== LLC SRAM READ test %s\n", msg);
	for (addr = PLAT_MARVELL_TRUSTED_RAM_BASE,
	     end_addr = PLAT_MARVELL_TRUSTED_RAM_BASE + size;
	     addr < end_addr; addr += 4) {
		data = mmio_read_32(addr);
		if (data != addr) {
			INFO("=== LLC SRAM READ test %s FAILED @ 0x%08lx)\n",
			     msg, addr);
			return -1;
		}
	}
	INFO("=== LLC SRAM READ test %s PASSED (last read = 0x%08x)\n",
	     msg, data);
	return 0;
}

#endif /* LLC_SRAM */

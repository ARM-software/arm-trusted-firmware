/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * GigaDevice-family FlexSPI NOR vendor helpers.
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <flash_vendor_gigadevice.h>
#include "fspi.h"
#include <fspi_api.h>
#include <lib/mmio.h>
#include <xspi_error_codes.h>

#include <platform_def.h>

/*
 * GD55 FlexSPI LUT slot allocations. These are reserved by the
 * vendor helpers and must not collide with the upstream
 * FSPI_*_SEQ_ID slots declared in fspi.h (0..9). 10..11 are left
 * free for future upstream additions; we use 12 and 13.
 */
#define GD_FSPI_LUT_VLUL_SEQ_ID		12U
#define GD_FSPI_LUT_RDLCK_SEQ_ID	13U

/* GD55 command opcodes (Table 12)
 * VL: Volatile Lock for every reboot
 * NL: Nonvolalite lock
 */
#define GD_CMD_GLOBAL_VL_UNLOCK		0x98U
#define GD_CMD_READ_VL_REGISTER		0xE0U
#define GD_CMD_READ_NL_REGISTER		0xE2U

/* fspi.c exposes these privately; we use the same MMIO abstraction
 * as the rest of the driver to keep endian-handling consistent.
 */
static uintptr_t gd_fspi_base;

static void gd_set_fspi_base(void)
{
	if (gd_fspi_base == 0U) {
		gd_fspi_base = (uintptr_t)NXP_FLEXSPI_ADDR;
	}
}

static inline void gd_writel(uint32_t off, uint32_t val)
{
	mmio_write_32(gd_fspi_base + off, val);
}

static inline uint32_t gd_readl(uint32_t off)
{
	return mmio_read_32(gd_fspi_base + off);
}

/*
 * FlexSPI LUT unlock/lock around the few entries we need to patch.
 * The key 0x5AF05AF0 (FSPI_LUTKEY_VALUE) must be written first to
 * FSPI_LUTKEY, then
 * FSPI_LCKCR = FSPI_LCKER_UNLOCK
 * to open the LUT window.
 *
 * Lock with the same key + FSPI_LCKER_LOCK.
 */
static void gd_lut_unlock(void)
{
	gd_writel(FSPI_LUTKEY, FSPI_LUTKEY_VALUE);
	gd_writel(FSPI_LCKCR, FSPI_LCKER_UNLOCK);
}

static void gd_lut_lock(void)
{
	gd_writel(FSPI_LUTKEY, FSPI_LUTKEY_VALUE);
	gd_writel(FSPI_LCKCR, FSPI_LCKER_LOCK);
}

/*
 * Build the 32-bit LUT instruction word (two 16-bit instruction
 * operations per DWORD, low-first). Matches the encoding in fspi.h's
 * FSPI_INSTR_* macros, but copied inline here so this file doesn't
 * depend on yet-another internal header.
 */
static inline uint32_t gd_lut_instr(uint8_t op0_opcode, uint8_t op0_pad,
				    uint8_t op0_operand,
				    uint8_t op1_opcode, uint8_t op1_pad,
				    uint8_t op1_operand)
{
	return ((uint32_t)op0_operand) |
	       ((uint32_t)op0_pad << 8) |
	       ((uint32_t)op0_opcode << 10) |
	       ((uint32_t)op1_operand << 16) |
	       ((uint32_t)op1_pad << 24) |
	       ((uint32_t)op1_opcode << 26);
}

/*
 * Trigger a LUT sequence and poll for IPCMDDONE. Returns 0 on
 * success, -EIO on IP command engine error.
 */
static int gd_trigger_and_wait(uint32_t seq_id, uint32_t addr,
			       uint32_t idatsz)
{
	uint32_t intr;

	gd_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	gd_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE);

	gd_writel(FSPI_IPCR0, addr);
	gd_writel(FSPI_IPCR1,
		  (seq_id << FSPI_IPCR1_SEQID_SHIFT) | idatsz);
	gd_writel(FSPI_IPCMD, FSPI_IPCMD_TRG);

	while ((gd_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE) == 0U) {
		/* spin */
	}
	intr = gd_readl(FSPI_INTR);
	gd_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE);

	if ((intr & (FSPI_INTR_IPCMDGE | FSPI_INTR_IPCMDERR)) != 0U)
		return -EIO;

	return 0;
}

/*
 * Common path for Read VL / Read NL: opcode + 32-bit address +
 * single data byte.
 */
static int gd_read_lock_register(uint8_t opcode, uint32_t offset,
				 uint8_t *val)
{
	uint32_t lut_addr;
	int ret;

	if (val == NULL)
		return -EINVAL;

	gd_set_fspi_base();

	lut_addr = FSPI_LUTREG_OFFSET +
		   0x10U * GD_FSPI_LUT_RDLCK_SEQ_ID;

	gd_lut_unlock();
	/* DWORD0: CMD(opcode, PAD1) | ADDR32BIT(PAD1) */
	gd_writel(lut_addr + 0x0U,
		  gd_lut_instr(FSPI_LUT_CMD, FSPI_LUT_PAD1, opcode,
			       FSPI_LUT_ADDR, FSPI_LUT_PAD1,
			       FSPI_LUT_ADDR32BIT));
	/* DWORD1: READ(1 byte, PAD1) | STOP */
	gd_writel(lut_addr + 0x4U,
		  gd_lut_instr(FSPI_LUT_READ, FSPI_LUT_PAD1, 1U,
			       0, 0, 0) & 0x0000FFFFU);
	gd_writel(lut_addr + 0x8U, 0U);
	gd_writel(lut_addr + 0xCU, 0U);
	gd_lut_lock();

	ret = gd_trigger_and_wait(GD_FSPI_LUT_RDLCK_SEQ_ID, offset, 1U);
	if (ret != 0)
		return ret;

	*val = (uint8_t)(gd_readl(FSPI_RFDR) & 0xFFU);

	/* Drain the RX FIFO so a subsequent read doesn't see stale data. */
	gd_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	gd_writel(FSPI_INTR, FSPI_INTR_IPRXWA);

	return 0;
}

int gd_global_vl_unlock(void)
{
	uint32_t lut_addr;
	int ret;

	gd_set_fspi_base();

	/* WREN is required to arm a volatile-lock change. */
	ret = xspi_wren(0U);
	if (ret != XSPI_SUCCESS) {
		ERROR("gd: WREN before GLOBAL_VL_UNLOCK failed (%d)\n", ret);
		return -EIO;
	}

	lut_addr = FSPI_LUTREG_OFFSET +
		   0x10U * GD_FSPI_LUT_VLUL_SEQ_ID;

	gd_lut_unlock();
	gd_writel(lut_addr + 0x0U,
		  gd_lut_instr(FSPI_LUT_CMD, FSPI_LUT_PAD1,
			       GD_CMD_GLOBAL_VL_UNLOCK,
			       0, 0, 0) & 0x0000FFFFU);
	gd_writel(lut_addr + 0x4U, 0U);
	gd_writel(lut_addr + 0x8U, 0U);
	gd_writel(lut_addr + 0xCU, 0U);
	gd_lut_lock();

	return gd_trigger_and_wait(GD_FSPI_LUT_VLUL_SEQ_ID, 0U, 0U);
}

int gd_read_vl_register(uint32_t offset, uint8_t *val)
{
	return gd_read_lock_register(GD_CMD_READ_VL_REGISTER, offset, val);
}

int gd_read_nl_register(uint32_t offset, uint8_t *val)
{
	return gd_read_lock_register(GD_CMD_READ_NL_REGISTER, offset, val);
}

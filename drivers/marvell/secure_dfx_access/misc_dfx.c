/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include "dfx.h"
#include <mvebu_def.h>
#include <mvebu.h>
#include <errno.h>

/* #define DEBUG_DFX */
#ifdef DEBUG_DFX
#define debug(format...) NOTICE(format)
#else
#define debug(format, arg...)
#endif

#define SAR_BASE			(MVEBU_REGS_BASE + 0x6F8200)
#define SAR_SIZE			0x4
#define AP_DEV_ID_STATUS_REG		(MVEBU_REGS_BASE + 0x6F8240)
#define JTAG_DEV_ID_STATUS_REG		(MVEBU_REGS_BASE + 0x6F8244)
#define EFUSE_CTRL			(MVEBU_REGS_BASE + 0x6F8008)
#define EFUSE_LD_BASE			(MVEBU_REGS_BASE + 0x6F8F00)
#define EFUSE_LD_SIZE			0x1C
#define EFUSE_HD_BASE			(MVEBU_REGS_BASE + 0x6F9000)
#define EFUSE_HD_SIZE			0x3F8

static _Bool is_valid(u_register_t addr)
{
	switch (addr) {
	case AP_DEV_ID_STATUS_REG:
	case JTAG_DEV_ID_STATUS_REG:
	case SAR_BASE ... (SAR_BASE + SAR_SIZE):
	case EFUSE_LD_BASE ... (EFUSE_LD_BASE + EFUSE_LD_SIZE):
	case EFUSE_HD_BASE ... (EFUSE_HD_BASE + EFUSE_HD_SIZE):
	case EFUSE_CTRL:
		return true;
	default:
		return false;
	}
}

static int armada_dfx_sread(u_register_t *read, u_register_t addr)
{
	if (!is_valid(addr))
		return -EINVAL;

	*read = mmio_read_32(addr);

	return 0;
}

static int armada_dfx_swrite(u_register_t addr, u_register_t val)
{
	if (!is_valid(addr))
		return -EINVAL;

	mmio_write_32(addr, val);

	return 0;
}

int mvebu_dfx_misc_handle(u_register_t func, u_register_t *read,
			  u_register_t addr, u_register_t val)
{
	debug_enter();

	debug("func %ld, addr 0x%lx, val 0x%lx\n", func, addr, val);

	switch (func) {
	case MV_SIP_DFX_SREAD:
		return armada_dfx_sread(read, addr);
	case MV_SIP_DFX_SWRITE:
		return armada_dfx_swrite(addr, val);
	default:
		ERROR("unsupported dfx misc sub-func\n");
		return -EINVAL;
	}

	debug_exit();

	return 0;
}

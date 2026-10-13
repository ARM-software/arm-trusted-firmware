/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <ele_api.h>
#include <platform_def.h>

#define ELE_MU_RSR	(ELE_MU_BASE + 0x12c)
#define ELE_MU_TRx(i)	(ELE_MU_BASE + 0x200 + (i) * 4)
#define ELE_MU_RRx(i)	(ELE_MU_BASE + 0x280 + (i) * 4)

#define ELE_MU_RSR_RX_MASK	U(0x3)
#define ELE_RESP_SUCCESS_MASK	U(0xff)
#define ELE_RESP_SUCCESS	U(0xd6)

static struct ele_soc_info soc_info;

int imx9_soc_info_handler(uint32_t smc_fid, void *handle)
{
	SMC_RET4(handle, 0x0, soc_info.soc,
		 soc_info.uid[1] | (uint64_t)soc_info.uid[0] << 32,
		 soc_info.uid[3] | (uint64_t)soc_info.uid[2] << 32);
}

void ele_get_soc_info(void)
{
	uint32_t msg, resp;

	flush_dcache_range((uint64_t)&soc_info, sizeof(struct ele_soc_info));

	mmio_write_32(ELE_MU_TRx(0), ELE_GET_INFO_REQ);
	mmio_write_32(ELE_MU_TRx(1), ((uint64_t) &soc_info) >> 32);
	mmio_write_32(ELE_MU_TRx(2), ((uint64_t) &soc_info) & 0xffffffff);
	mmio_write_32(ELE_MU_TRx(3), sizeof(struct ele_soc_info));

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & ELE_MU_RSR_RX_MASK) != ELE_MU_RSR_RX_MASK);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);

	/* Ensure write to soc_info is visible before CPU reads it */
	dsb();

#if defined(PLAT_imx95)
	soc_info.soc &= 0xFFFFFF00;
#endif
}

void ele_release_gmid(void)
{
	uint32_t msg, resp;

	mmio_write_32(ELE_MU_TRx(0), ELE_RELEASE_GMID);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & ELE_MU_RSR_RX_MASK) != ELE_MU_RSR_RX_MASK);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);
}

void ele_request_pwrdown(void)
{
	uint32_t msg, resp;
	unsigned int timeout = 10000U;

	mmio_write_32(ELE_MU_TRx(0), ELE_POWER_DOWN_REQ);
	mmio_write_32(ELE_MU_TRx(1), 0x4000);
	mmio_write_32(ELE_MU_TRx(2), BL31_BASE);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
		if ((resp & ELE_MU_RSR_RX_MASK) == ELE_MU_RSR_RX_MASK) {
			break;
		}
		udelay(1);
	} while (--timeout > 0U);

	if (timeout == 0U) {
		ERROR("ELE power-down request timeout\n");
		return;
	}

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));

	VERBOSE("resp %x; %x\n", msg, resp);
	if ((resp & ELE_RESP_SUCCESS_MASK) != ELE_RESP_SUCCESS) {
		WARN("ELE power-down request failed: resp=0x%x\n", resp);
	}
}

uint32_t ele_read_common_fuse(uint32_t fuse_id)
{
	uint32_t msg, resp, val = 0;
	unsigned int timeout = 10000U; /* ~10ms */

	mmio_write_32(ELE_MU_TRx(0), ELE_READ_FUSE_REQ);
	mmio_write_32(ELE_MU_TRx(1), fuse_id);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
		if ((resp & ELE_MU_RSR_RX_MASK) == ELE_MU_RSR_RX_MASK) {
			break;
		}
		udelay(1);
	} while (--timeout > 0U);

	if (timeout == 0U) {
		ERROR("ELE MU response timeout for fuse %u\n", fuse_id);
		return 0U;
	}

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));

	if ((resp & ELE_RESP_SUCCESS_MASK) == ELE_RESP_SUCCESS) {
		val = mmio_read_32(ELE_MU_RRx(2));
	} else {
		WARN("ELE fuse read failed: resp=0x%x\n", resp);
	}

	VERBOSE("resp %x; %x; %x\n", msg, resp, val);

	return val;
}

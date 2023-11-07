/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <ele_api.h>
#include <platform_def.h>

#define ELE_MU_RSR	(ELE_MU_BASE + 0x12c)
#define ELE_MU_TRx(i)	(ELE_MU_BASE + 0x200 + (i) * 4)
#define ELE_MU_RRx(i)	(ELE_MU_BASE + 0x280 + (i) * 4)

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
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);
}

void ele_release_gmid(void)
{
	uint32_t msg, resp;

	mmio_write_32(ELE_MU_TRx(0), ELE_RELEASE_GMID);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);
}

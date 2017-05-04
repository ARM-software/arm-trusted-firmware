/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <mmio.h>
#include <plat_sip_calls.h>
#include <rockchip_sip_svc.h>
#include <runtime_svc.h>
#include <dfs.h>

#define RK_SIP_DDR_CFG		0x82000008
#define DRAM_INIT		0x00
#define DRAM_SET_RATE		0x01
#define DRAM_ROUND_RATE		0x02
#define DRAM_SET_AT_SR		0x03
#define DRAM_GET_BW		0x04
#define DRAM_GET_RATE		0x05
#define DRAM_CLR_IRQ		0x06
#define DRAM_SET_PARAM		0x07
#define DRAM_SET_ODT_PD		0x08

uint32_t ddr_smc_handler(uint64_t arg0, uint64_t arg1,
			 uint64_t id, uint64_t arg2)
{
	switch (id) {
	case DRAM_SET_RATE:
		return ddr_set_rate((uint32_t)arg0);
	case DRAM_ROUND_RATE:
		return ddr_round_rate((uint32_t)arg0);
	case DRAM_GET_RATE:
		return ddr_get_rate();
	case DRAM_SET_ODT_PD:
		dram_set_odt_pd(arg0, arg1, arg2);
		break;
	default:
		break;
	}

	return 0;
}

uint64_t rockchip_plat_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	switch (smc_fid) {
	case RK_SIP_DDR_CFG:
		SMC_RET1(handle, ddr_smc_handler(x1, x2, x3, x4));
	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

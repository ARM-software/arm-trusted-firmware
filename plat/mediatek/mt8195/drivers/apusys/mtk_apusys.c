/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>

#include <apupwr_clkctl.h>
#include <mtk_apusys.h>
#include <plat/common/platform.h>

int32_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
			    uint32_t *ret1)
{
	int32_t ret = 0L;
	uint32_t request_ops;

	request_ops = (uint32_t)x1;

	switch (request_ops) {
	case MTK_SIP_APU_START_MCU:
		/* setup addr[33:32] in reviser */
		mmio_write_32(REVISER_SECUREFW_CTXT, 0U);
		mmio_write_32(REVISER_USDRFW_CTXT, 0U);

		/* setup secure sideband */
		mmio_write_32(AO_SEC_FW,
			      (SEC_FW_NON_SECURE << SEC_FW_SHIFT_NS) |
			      (0U << SEC_FW_DOMAIN_SHIFT));

		/* setup boot address */
		mmio_write_32(AO_MD32_BOOT_CTRL, 0U);

		/* setup pre-define region */
		mmio_write_32(AO_MD32_PRE_DEFINE,
			      (PRE_DEFINE_CACHE_TCM << PRE_DEFINE_SHIFT_0G) |
			      (PRE_DEFINE_CACHE << PRE_DEFINE_SHIFT_1G) |
			      (PRE_DEFINE_CACHE << PRE_DEFINE_SHIFT_2G) |
			      (PRE_DEFINE_CACHE << PRE_DEFINE_SHIFT_3G));

		/* release runstall */
		mmio_write_32(AO_MD32_SYS_CTRL, SYS_CTRL_RUN);

		INFO("[APUSYS] rev(0x%08x,0x%08x)\n",
		     mmio_read_32(REVISER_SECUREFW_CTXT),
		     mmio_read_32(REVISER_USDRFW_CTXT));
		INFO("[APUSYS] ao(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x)\n",
		     mmio_read_32(AO_SEC_FW),
		     mmio_read_32(AO_SEC_USR_FW),
		     mmio_read_32(AO_MD32_BOOT_CTRL),
		     mmio_read_32(AO_MD32_PRE_DEFINE),
		     mmio_read_32(AO_MD32_SYS_CTRL));
		break;
	case MTK_SIP_APU_STOP_MCU:
		/* hold runstall */
		mmio_write_32(AO_MD32_SYS_CTRL, SYS_CTRL_STALL);

		INFO("[APUSYS] md32_boot_ctrl=0x%08x,runstall=0x%08x\n",
		     mmio_read_32(AO_MD32_BOOT_CTRL),
		     mmio_read_32(AO_MD32_SYS_CTRL));
		break;
	case MTK_SIP_APUPWR_BUS_PROT_CG_ON:
		apupwr_smc_bus_prot_cg_on();
		break;
	case MTK_SIP_APUPWR_BULK_PLL:
		ret = apupwr_smc_bulk_pll((bool)x2);
		break;
	case MTK_SIP_APUPWR_ACC_INIT_ALL:
		ret = apupwr_smc_acc_init_all();
		break;
	case MTK_SIP_APUPWR_ACC_TOP:
		apupwr_smc_acc_top((bool)x2);
		break;
	default:
		ERROR("%s, unknown request_ops=0x%x\n", __func__, request_ops);
		break;
	}

	return ret;
}

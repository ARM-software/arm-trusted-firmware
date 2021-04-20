/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <mtk_apusys.h>
#include <plat/common/platform.h>

uint64_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
			    uint32_t *ret1)
{
	uint32_t request_ops;

	request_ops = (uint32_t)x1;
	INFO("[APUSYS] ops=0x%x\n", request_ops);

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

		INFO("[APUSYS] reviser_ctxt=%x,%x\n",
		     mmio_read_32(REVISER_SECUREFW_CTXT),
		     mmio_read_32(REVISER_USDRFW_CTXT));
		INFO("[APUSYS]fw=0x%08x,boot=0x%08x,def=0x%08x,sys=0x%08x\n",
		     mmio_read_32(AO_SEC_FW),
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
	default:
		ERROR("%s, unknown request_ops = %x\n", __func__, request_ops);
		break;
	}

	return 0UL;
}

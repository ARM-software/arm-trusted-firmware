/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_APUSYS_H
#define MTK_APUSYS_H

#include <stdint.h>

/* setup the SMC command ops */
#define MTK_SIP_APU_START_MCU		(0x00U)
#define MTK_SIP_APU_STOP_MCU		(0x01U)
#define MTK_SIP_APUPWR_BUS_PROT_CG_ON	(0x02U)
#define MTK_SIP_APUPWR_BULK_PLL		(0x03U)
#define MTK_SIP_APUPWR_ACC_INIT_ALL	(0x04U)
#define MTK_SIP_APUPWR_ACC_TOP		(0x05U)

/* AO Register */
#define AO_MD32_PRE_DEFINE	(APUSYS_APU_S_S_4_BASE + 0x00)
#define AO_MD32_BOOT_CTRL	(APUSYS_APU_S_S_4_BASE + 0x04)
#define AO_MD32_SYS_CTRL	(APUSYS_APU_S_S_4_BASE + 0x08)
#define AO_SEC_FW		(APUSYS_APU_S_S_4_BASE + 0x10)
#define AO_SEC_USR_FW		(APUSYS_APU_S_S_4_BASE + 0x14)

#define PRE_DEFINE_CACHE_TCM	(0x3U)
#define PRE_DEFINE_CACHE	(0x2U)
#define PRE_DEFINE_SHIFT_0G	(0U)
#define PRE_DEFINE_SHIFT_1G	(2U)
#define PRE_DEFINE_SHIFT_2G	(4U)
#define PRE_DEFINE_SHIFT_3G	(6U)

#define SEC_FW_NON_SECURE	(1U)
#define SEC_FW_SHIFT_NS		(4U)
#define SEC_FW_DOMAIN_SHIFT	(0U)

#define SEC_USR_FW_NON_SECURE	(1U)
#define SEC_USR_FW_SHIFT_NS	(4U)
#define SEC_USR_FW_DOMAIN_SHIFT	(0U)

#define SYS_CTRL_RUN		(0U)
#define SYS_CTRL_STALL		(1U)

/* Reviser Register */
#define REVISER_SECUREFW_CTXT	(APUSYS_SCTRL_REVISER_BASE + 0x100)
#define REVISER_USDRFW_CTXT	(APUSYS_SCTRL_REVISER_BASE + 0x104)

int32_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
			   uint32_t *ret1);
#endif /* MTK_APUSYS_H */

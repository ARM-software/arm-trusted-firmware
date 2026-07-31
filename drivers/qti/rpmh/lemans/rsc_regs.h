/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RSC (Resource State Coordinator) register definitions for the lemans
 * (qcs9075) APSS RPMh controller, DRV0 (TZ) view.
 */

#ifndef RSC_REGS_H
#define RSC_REGS_H

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/*
 * APSS RSC register base. Derived from the HW description:
 *   APSS_RSC_RSCC_RSCC_RSC_REG_BASE = QTI_APSS_HM_BASE + 0x00a00000
 */
#define RSC_REG_BASE			(QTI_APSS_HM_BASE + 0x00a00000U)

/* Per-DRV AMC-mode IRQ registers (DRV0). */
#define RSC_AMC_IRQ_ENABLE_OFF		0xd00U
#define RSC_AMC_IRQ_STATUS_OFF		0xd04U
#define RSC_AMC_IRQ_CLEAR_OFF		0xd08U

/* Per-TCS register block within DRV0. */
#define RSC_TCS_STRIDE			0x2a0U
#define RSC_TCS_BASE_OFF		0xd10U

#define RSC_TCS_CMD_WAIT_FOR_CMPL_OFF	0x00U
#define RSC_TCS_CONTROL_OFF		0x04U
#define RSC_TCS_STATUS_OFF		0x08U
#define RSC_TCS_CMD_ENABLE_OFF		0x0cU

/* CONTROL register fields. */
#define RSC_TCS_CONTROL_AMC_MODE_TRIGGER	BIT(24)
#define RSC_TCS_CONTROL_AMC_MODE_EN		BIT(16)

/* STATUS register fields. */
#define RSC_TCS_STATUS_CONTROLLER_IDLE		BIT(0)

/* Per-command register block within a TCS. */
#define RSC_CMD_STRIDE			0x14U
#define RSC_CMD_BASE_OFF		0xd30U

#define RSC_CMD_MSGID_OFF		0x00U
#define RSC_CMD_ADDR_OFF		0x04U
#define RSC_CMD_DATA_OFF		0x08U
#define RSC_CMD_STATUS_OFF		0x0cU
#define RSC_CMD_RESP_DATA_OFF		0x10U

/* MSGID register fields. */
#define RSC_CMD_MSGID_READ_OR_WRITE	BIT(16)
#define RSC_CMD_MSGID_RES_REQ		BIT(8)
#define RSC_CMD_MSGID_MSG_LENGTH_SHFT	0U
#define RSC_CMD_MSGID_MSG_LENGTH_MASK	0xfU

/* ADDR register fields. */
#define RSC_CMD_ADDR_SLV_ID_SHFT	16U
#define RSC_CMD_ADDR_SLV_ID_MASK	0x7U
#define RSC_CMD_ADDR_OFFSET_SHFT	0U
#define RSC_CMD_ADDR_OFFSET_MASK	0xffffU

/* CMD STATUS register fields. */
#define RSC_CMD_STATUS_COMPLETED	BIT(16)
#define RSC_CMD_STATUS_ISSUED		BIT(8)
#define RSC_CMD_STATUS_TRIGGERED	BIT(0)

/* Each RPMh command transfers a single 32-bit word (encoded as 8 bytes). */
#define RSC_CMD_DATA_LEN		8U

/* Split a {slave-id, offset} resource address. */
#define RSC_SLAVE_ID(addr)		(((addr) >> 16) & RSC_CMD_ADDR_SLV_ID_MASK)
#define RSC_OFFSET(addr)		((addr) & RSC_CMD_ADDR_OFFSET_MASK)

/* Address helpers (DRV0). */
#define RSC_DRV0_REG(off) \
	((uintptr_t)RSC_REG_BASE + (uint32_t)(off))

#define RSC_TCS_REG(tcs, off) \
	RSC_DRV0_REG(RSC_TCS_BASE_OFF + (RSC_TCS_STRIDE * (uint32_t)(tcs)) + \
		     (uint32_t)(off))

#define RSC_CMD_REG(tcs, cmd, off) \
	RSC_DRV0_REG(RSC_CMD_BASE_OFF + (RSC_TCS_STRIDE * (uint32_t)(tcs)) + \
		     (RSC_CMD_STRIDE * (uint32_t)(cmd)) + (uint32_t)(off))

#endif /* RSC_REGS_H */

/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <pmif.h>
#include "pmif_common.h"
#include "spmi_common.h"
#include "spmi_sw.h"

#define PMIF_CMD_REG_0			0
#define PMIF_CMD_REG			1
#define PMIF_CMD_EXT_REG		2
#define PMIF_CMD_EXT_REG_LONG		3
#define PMIF_READ_CMD_MIN		0x60
#define PMIF_READ_CMD_MAX		0x7F
#define PMIF_READ_CMD_EXT_MIN		0x20
#define PMIF_READ_CMD_EXT_MAX		0x2F
#define PMIF_READ_CMD_EXT_LONG_MIN	0x38
#define PMIF_READ_CMD_EXT_LONG_MAX	0x3F
#define PMIF_WRITE_CMD_MIN		0x40
#define PMIF_WRITE_CMD_MAX		0x5F
#define PMIF_WRITE_CMD_EXT_MAX		0xF
#define PMIF_WRITE_CMD_EXT_LONG_MIN	0x30
#define PMIF_WRITE_CMD_EXT_LONG_MAX	0x37
#define PMIF_WRITE_CMD_0_MIN		0x80

/* macro for SWINF_FSM */
#define SWINF_FSM_IDLE			0x00
#define SWINF_FSM_REQ			0x02
#define SWINF_FSM_WFDLE			0x04
#define SWINF_FSM_WFVLDCLR		0x06

#define GET_SWINF_FSM(x)		(((x) >> 1) & 0x7)
#define GET_PMIF_INIT_DONE(x)		(((x) >> 15) & 0x1)
#define TIMEOUT_WAIT_IDLE_US		10000 /* 10ms */

#define PMIF_RW_CMD_SET(opc, rw, sid, bc, addr)	\
	(((opc) << 30) | ((rw) << 29) | ((sid) << 24) | ((bc) << 16) | (addr))

static spinlock_t pmif_lock;

struct pmif *get_pmif_controller(int inf, int mstid)
{
	return &pmif_spmi_arb[mstid];
}

static int pmif_check_idle(int mstid)
{
	struct pmif *arb = get_pmif_controller(PMIF_SPMI, mstid);
	unsigned int reg_rdata, offset = 0;

	do {
		offset = arb->regs[PMIF_SWINF_3_STA];
		reg_rdata = mmio_read_32((uintptr_t)(arb->base + offset));
	} while (GET_SWINF_FSM(reg_rdata) != SWINF_FSM_IDLE);

	return 0;
}

static int pmif_check_vldclr(int mstid)
{
	struct pmif *arb = get_pmif_controller(PMIF_SPMI, mstid);
	unsigned int reg_rdata, offset = 0;

	do {
		offset = arb->regs[PMIF_SWINF_3_STA];
		reg_rdata = mmio_read_32((uintptr_t)(arb->base + offset));
	} while (GET_SWINF_FSM(reg_rdata) != SWINF_FSM_WFVLDCLR);

	return 0;
}

int pmif_spmi_read_cmd(struct pmif *arb, uint8_t opc, uint8_t sid,
		       uint16_t addr, uint8_t *buf, uint8_t len)
{
	int ret;
	uint32_t offset = 0, data = 0;
	uint8_t bc = len - 1;

	if (sid > SPMI_MAX_SLAVE_ID || len > PMIF_BYTECNT_MAX)
		return -EINVAL;

	/* Check the opcode */
	if (opc >= PMIF_READ_CMD_MIN && opc <= PMIF_READ_CMD_MAX)
		opc = PMIF_CMD_REG;
	else if (opc >= PMIF_READ_CMD_EXT_MIN && opc <= PMIF_READ_CMD_EXT_MAX)
		opc = PMIF_CMD_EXT_REG;
	else if (opc >= PMIF_READ_CMD_EXT_LONG_MIN && opc <= PMIF_READ_CMD_EXT_LONG_MAX)
		opc = PMIF_CMD_EXT_REG_LONG;
	else
		return -EINVAL;

	spin_lock(&pmif_lock);

	/* Wait for Software Interface FSM state to be IDLE. */
	ret = pmif_check_idle(arb->mstid);
	if (ret)
		goto done;

	/* Send the command. */
	offset = arb->regs[PMIF_SWINF_3_ACC];
	mmio_write_32((uintptr_t)(arb->base + offset), PMIF_RW_CMD_SET(opc, 0, sid, bc, addr));
	/*
	 * Wait for Software Interface FSM state to be WFVLDCLR,
	 * read the data and clear the valid flag.
	 */
	ret = pmif_check_vldclr(arb->mstid);
	if (ret)
		goto done;

	offset = arb->regs[PMIF_SWINF_3_RDATA_31_0];

	data = mmio_read_32((uintptr_t)(arb->base + offset));
	memcpy(buf, &data, (bc & 3) + 1);

	offset = arb->regs[PMIF_SWINF_3_VLD_CLR];
	mmio_write_32((uintptr_t)(arb->base + offset), 0x1);

done:
	spin_unlock(&pmif_lock);
	return ret;
}

int pmif_spmi_write_cmd(struct pmif *arb, uint8_t opc, uint8_t sid, uint16_t addr,
			const uint8_t *buf, uint8_t len)
{
	int ret;
	uint32_t offset = 0, data = 0;
	uint8_t bc = len - 1;

	if (sid > SPMI_MAX_SLAVE_ID || len > PMIF_BYTECNT_MAX)
		return -EINVAL;

	/* Check the opcode */
	if (opc >= PMIF_WRITE_CMD_MIN && opc <= PMIF_WRITE_CMD_MAX)
		opc = PMIF_CMD_REG;
	else if (opc <= PMIF_WRITE_CMD_EXT_MAX)
		opc = PMIF_CMD_EXT_REG;
	else if (opc >= PMIF_WRITE_CMD_EXT_LONG_MIN && opc <= PMIF_WRITE_CMD_EXT_LONG_MAX)
		opc = PMIF_CMD_EXT_REG_LONG;
	else if (opc >= PMIF_WRITE_CMD_0_MIN)
		opc = PMIF_CMD_REG_0;
	else
		return -EINVAL;

	spin_lock(&pmif_lock);

	/* Wait for Software Interface FSM state to be IDLE. */
	ret = pmif_check_idle(arb->mstid);
	if (ret)
		goto done;

	/* Set the write data. */
	offset = arb->regs[PMIF_SWINF_3_WDATA_31_0];
	memcpy(&data, buf, (bc & 3) + 1);
	mmio_write_32((uintptr_t)(arb->base + offset), data);
	/* Send the command. */
	offset = arb->regs[PMIF_SWINF_3_ACC];
	mmio_write_32((uintptr_t)(arb->base + offset), PMIF_RW_CMD_SET(opc, 1, sid, bc, addr));

done:
	spin_unlock(&pmif_lock);
	return ret;
}

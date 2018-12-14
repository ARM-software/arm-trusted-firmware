/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <hi3660.h>
#include <hisi_ipc.h>
#include "../../hikey960_private.h"

#define IPC_MBX_SOURCE_REG(m)		(IPC_BASE + ((m) << 6))
#define IPC_MBX_DSET_REG(m)		(IPC_BASE + ((m) << 6) + 0x04)
#define IPC_MBX_DCLEAR_REG(m)		(IPC_BASE + ((m) << 6) + 0x08)
#define IPC_MBX_DSTATUS_REG(m)		(IPC_BASE + ((m) << 6) + 0x0C)
#define IPC_MBX_MODE_REG(m)		(IPC_BASE + ((m) << 6) + 0x10)
#define IPC_MBX_IMASK_REG(m)		(IPC_BASE + ((m) << 6) + 0x14)
#define IPC_MBX_ICLR_REG(m)		(IPC_BASE + ((m) << 6) + 0x18)
#define IPC_MBX_SEND_REG(m)		(IPC_BASE + ((m) << 6) + 0x1C)
#define IPC_MBX_DATA_REG(m, d)		(IPC_BASE + ((m) << 6) + 0x20 + \
					 ((d) * 4))
#define IPC_CPU_IMST_REG(m)		(IPC_BASE + ((m) << 3))
#define IPC_LOCK_REG			(IPC_BASE + 0xA00)
#define IPC_ACK_BIT_SHIFT		(1 << 7)
#define IPC_UNLOCK_VALUE		(0x1ACCE551)

/*********************************************************
 *bit[31:24]:0~AP
 *bit[23:16]:0x1~A15, 0x2~A7
 *bit[15:8]:0~ON, 1~OFF
 *bit[7:0]:0x3 cpu power mode
 *********************************************************/
#define IPC_CMD_TYPE(src_obj, cluster_obj, is_off, mode) \
	((src_obj << 24) | (((cluster_obj) + 1) << 16) | (is_off << 8) | (mode))

/*********************************************************
 *bit[15:8]:0~no idle, 1~idle
 *bit[7:0]:cpux
 *********************************************************/

#define IPC_CMD_PARA(is_idle, cpu) \
	((is_idle << 8) | (cpu))

#define IPC_STATE_IDLE			0x10

enum src_id {
	SRC_IDLE = 0,
	SRC_A15 = 1 << 0,
	SRC_A7 = 1 << 1,
	SRC_IOM3 = 1 << 2,
	SRC_LPM3 = 1 << 3
};

/*lpm3's mailboxs are 13~17*/
enum lpm3_mbox_id {
	LPM3_MBX0 = 13,
	LPM3_MBX1,
	LPM3_MBX2,
	LPM3_MBX3,
	LPM3_MBX4,
};

static void cpu_relax(void)
{
	volatile int i;

	for (i = 0; i < 10; i++)
		nop();
}

static inline void
hisi_ipc_clear_ack(enum src_id source, enum lpm3_mbox_id mbox)
{
	unsigned int int_status = 0;

	do {
		int_status = mmio_read_32(IPC_MBX_MODE_REG(mbox));
		int_status &= 0xF0;
		cpu_relax();
	} while (int_status != IPC_ACK_BIT_SHIFT);

	mmio_write_32(IPC_MBX_ICLR_REG(mbox), source);
}

static void
hisi_ipc_send_cmd_with_ack(enum src_id source, enum lpm3_mbox_id mbox,
			   unsigned int cmdtype, unsigned int cmdpara)
{
	unsigned int regval;
	unsigned int mask;
	unsigned int state;

	mmio_write_32(IPC_LOCK_REG, IPC_UNLOCK_VALUE);
	/* wait for idle and occupy */
	do {
		state = mmio_read_32(IPC_MBX_MODE_REG(mbox));
		if (state == IPC_STATE_IDLE) {
			mmio_write_32(IPC_MBX_SOURCE_REG(mbox), source);
			regval = mmio_read_32(IPC_MBX_SOURCE_REG(mbox));
			if (regval == source)
				break;
		}
		cpu_relax();

	} while (1);

	/* auto answer */
	mmio_write_32(IPC_MBX_MODE_REG(mbox), 0x1);

	mask = (~((int)source | SRC_LPM3) & 0x3F);
	/* mask the other cpus */
	mmio_write_32(IPC_MBX_IMASK_REG(mbox), mask);
	/* set data */
	mmio_write_32(IPC_MBX_DATA_REG(mbox, 0), cmdtype);
	mmio_write_32(IPC_MBX_DATA_REG(mbox, 1), cmdpara);
	/* send cmd */
	mmio_write_32(IPC_MBX_SEND_REG(mbox), source);
	/* wait ack and clear */
	hisi_ipc_clear_ack(source, mbox);

	/* release mailbox */
	mmio_write_32(IPC_MBX_SOURCE_REG(mbox), source);
}

void hisi_ipc_pm_on_off(unsigned int core, unsigned int cluster,
			enum pm_mode mode)
{
	unsigned int cmdtype = 0;
	unsigned int cmdpara = 0;
	enum src_id source = SRC_IDLE;
	enum lpm3_mbox_id mailbox = (enum lpm3_mbox_id)(LPM3_MBX0 + core);

	cmdtype = IPC_CMD_TYPE(0, cluster, mode, 0x3);
	cmdpara = IPC_CMD_PARA(0, core);
	source = cluster ? SRC_A7 : SRC_A15;
	hisi_ipc_send_cmd_with_ack(source, mailbox, cmdtype, cmdpara);
}

void hisi_ipc_pm_suspend(unsigned int core, unsigned int cluster,
			 unsigned int affinity_level)
{
	unsigned int cmdtype = 0;
	unsigned int cmdpara = 0;
	enum src_id source = SRC_IDLE;
	enum lpm3_mbox_id mailbox = (enum lpm3_mbox_id)(LPM3_MBX0 + core);

	if (affinity_level == 0x3)
		cmdtype = IPC_CMD_TYPE(0, -1, 0x1, 0x3 + affinity_level);
	else
		cmdtype = IPC_CMD_TYPE(0, cluster, 0x1, 0x3 + affinity_level);

	cmdpara = IPC_CMD_PARA(1, core);
	source = cluster ? SRC_A7 : SRC_A15;
	hisi_ipc_send_cmd_with_ack(source, mailbox, cmdtype, cmdpara);
}

void hisi_ipc_psci_system_off(unsigned int core, unsigned int cluster)
{
	unsigned int cmdtype = 0;
	unsigned int cmdpara = 0;
	enum src_id source = SRC_IDLE;
	enum lpm3_mbox_id mailbox = (enum lpm3_mbox_id)(LPM3_MBX0 + core);

	cmdtype = IPC_CMD_TYPE(0, (0x10 - 1), 0x1, 0x0);
	cmdpara = IPC_CMD_PARA(0, 0);
	source = cluster ? SRC_A7 : SRC_A15;
	hisi_ipc_send_cmd_with_ack(source, mailbox, cmdtype, cmdpara);
}

void hisi_ipc_psci_system_reset(unsigned int core, unsigned int cluster,
				unsigned int cmd_id)
{
	unsigned int cmdtype = 0;
	unsigned int cmdpara = 0;
	enum src_id source = SRC_IDLE;
	enum lpm3_mbox_id mailbox = (enum lpm3_mbox_id)(LPM3_MBX0 + core);

	cmdtype = IPC_CMD_TYPE(0, (0x10 - 1), 0x0, 0x0);
	cmdpara = cmd_id;
	source = cluster ? SRC_A7 : SRC_A15;
	hisi_ipc_send_cmd_with_ack(source, mailbox, cmdtype, cmdpara);
}

int hisi_ipc_init(void)
{
	int ret = 0;
	enum lpm3_mbox_id  i = LPM3_MBX0;

	mmio_write_32(IPC_LOCK_REG, IPC_UNLOCK_VALUE);
	for (i = LPM3_MBX0; i <= LPM3_MBX4; i++) {
		mmio_write_32(IPC_MBX_MODE_REG(i), 1);
		mmio_write_32(IPC_MBX_IMASK_REG(i),
			      ((int)SRC_IOM3 | (int)SRC_A15 | (int)SRC_A7));
		mmio_write_32(IPC_MBX_ICLR_REG(i), SRC_A7);
	}

	return ret;
}

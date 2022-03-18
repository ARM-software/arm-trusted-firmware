/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORSTONE700_MHU_H
#define CORSTONE700_MHU_H

#define MHU_POLL_INTR_STAT_TIMEOUT		50000 /*timeout value in us*/

/* CPU MHU secure channel registers */
#define CPU_INTR_S_STAT				0x00
#define CPU_INTR_S_SET				0x0C

/* MHUv2 Control Registers Offsets */
#define MHU_V2_MSG_CFG_OFFSET			0xF80
#define MHU_V2_ACCESS_REQ_OFFSET		0xF88
#define MHU_V2_ACCESS_READY_OFFSET		0xF8C

#define MHU_V2_ACCESS_REQUEST(addr)     \
	mmio_write_32((addr) + MHU_V2_ACCESS_REQ_OFFSET, 0x1)

#define MHU_V2_CLEAR_REQUEST(addr)      \
	mmio_write_32((addr) + MHU_V2_ACCESS_REQ_OFFSET, 0x0)

#define MHU_V2_IS_ACCESS_READY(addr)    \
	(mmio_read_32((addr) + MHU_V2_ACCESS_READY_OFFSET) & 0x1)

void mhu_secure_message_start(uintptr_t address, unsigned int slot_id);
void mhu_secure_message_send(uintptr_t address,
				unsigned int slot_id,
				unsigned int message);
void mhu_secure_message_end(uintptr_t address, unsigned int slot_id);
void mhu_secure_init(void);

#endif /* CORSTONE700_MHU_H */

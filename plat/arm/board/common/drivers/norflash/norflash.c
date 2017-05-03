/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <mmio.h>
#include <norflash.h>

/* Helper macros to access two flash banks in parallel */
#define NOR_2X16(d)			((d << 16) | (d & 0xffff))

/*
 * DWS ready poll retries. The number of retries in this driver have been
 * obtained empirically from Juno. FVP implements a zero wait state NOR flash
 * model
 */
#define DWS_WORD_PROGRAM_RETRIES	1000

/*
 * Poll Write State Machine. Return values:
 *    0      = WSM ready
 *    -EBUSY = WSM busy after the number of retries
 */
static int nor_poll_dws(uintptr_t base_addr, unsigned int retries)
{
	uint32_t status;
	int ret;

	for (;;) {
		nor_send_cmd(base_addr, NOR_CMD_READ_STATUS_REG);
		status = mmio_read_32(base_addr);
		if ((status & NOR_DWS) &&
		    (status & (NOR_DWS << 16))) {
			ret = 0;
			break;
		}
		if (retries-- == 0) {
			ret = -EBUSY;
			break;
		}
	}

	return ret;
}

void nor_send_cmd(uintptr_t base_addr, unsigned long cmd)
{
	mmio_write_32(base_addr, NOR_2X16(cmd));
}

/*
 * Return values:
 *    0      = success
 *    -EBUSY = WSM not ready
 *    -EPERM = Device protected or Block locked
 */
int nor_word_program(uintptr_t base_addr, unsigned long data)
{
	uint32_t status;
	int ret;

	/* Set the device in write word mode */
	nor_send_cmd(base_addr, NOR_CMD_WORD_PROGRAM);
	mmio_write_32(base_addr, data);

	ret = nor_poll_dws(base_addr, DWS_WORD_PROGRAM_RETRIES);
	if (ret != 0) {
		goto word_program_end;
	}

	/* Full status check */
	nor_send_cmd(base_addr, NOR_CMD_READ_STATUS_REG);
	status = mmio_read_32(base_addr);

	if (status & (NOR_PS | NOR_BLS)) {
		nor_send_cmd(base_addr, NOR_CMD_CLEAR_STATUS_REG);
		ret = -EPERM;
	}

word_program_end:
	nor_send_cmd(base_addr, NOR_CMD_READ_ARRAY);
	return ret;
}

void nor_lock(uintptr_t base_addr)
{
	nor_send_cmd(base_addr, NOR_CMD_LOCK_UNLOCK);
	mmio_write_32(base_addr, NOR_2X16(NOR_LOCK_BLOCK));
	nor_send_cmd(base_addr, NOR_CMD_READ_ARRAY);
}

void nor_unlock(uintptr_t base_addr)
{
	nor_send_cmd(base_addr, NOR_CMD_LOCK_UNLOCK);
	mmio_write_32(base_addr, NOR_2X16(NOR_UNLOCK_BLOCK));
	nor_send_cmd(base_addr, NOR_CMD_READ_ARRAY);
}


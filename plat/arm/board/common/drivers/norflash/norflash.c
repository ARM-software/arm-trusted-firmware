/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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


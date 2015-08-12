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

#ifndef __NORFLASH_H_
#define __NORFLASH_H_

#include <stdint.h>

/* First bus cycle */
#define NOR_CMD_READ_ARRAY		0xFF
#define NOR_CMD_READ_ID_CODE		0x90
#define NOR_CMD_READ_QUERY		0x98
#define NOR_CMD_READ_STATUS_REG		0x70
#define NOR_CMD_CLEAR_STATUS_REG	0x50
#define NOR_CMD_WRITE_TO_BUFFER		0xE8
#define NOR_CMD_WORD_PROGRAM		0x40
#define NOR_CMD_BLOCK_ERASE		0x20
#define NOR_CMD_LOCK_UNLOCK		0x60

/* Second bus cycle */
#define NOR_LOCK_BLOCK			0x01
#define NOR_UNLOCK_BLOCK		0xD0

/* Status register bits */
#define NOR_DWS				(1 << 7)
#define NOR_ESS				(1 << 6)
#define NOR_ES				(1 << 5)
#define NOR_PS				(1 << 4)
#define NOR_VPPS			(1 << 3)
#define NOR_PSS				(1 << 2)
#define NOR_BLS				(1 << 1)
#define NOR_BWS				(1 << 0)

/* Public API */
void nor_send_cmd(uintptr_t base_addr, unsigned long cmd);
int nor_word_program(uintptr_t base_addr, unsigned long data);
void nor_lock(uintptr_t base_addr);
void nor_unlock(uintptr_t base_addr);

#endif /* __NORFLASH_H_ */


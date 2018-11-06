/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef V2M_FLASH_H
#define V2M_FLASH_H

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
#define NOR_CMD_BLOCK_ERASE_ACK		0xD0

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
int nor_lock(uintptr_t base_addr);
int nor_unlock(uintptr_t base_addr);
int nor_erase(uintptr_t base_addr);

#endif /* V2M_FLASH_H*/

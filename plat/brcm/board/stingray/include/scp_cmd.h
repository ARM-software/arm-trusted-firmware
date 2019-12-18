/*
 * Copyright (c) 2017-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_CMD_H
#define SCP_SMD_H

#include <stdint.h>

typedef struct {
	int cmd;
	int completed;
	int ret;
} crmu_response_t;


#define SCP_CMD_MASK 0xffff
#define SCP_CMD_DEFAULT_TIMEOUT_US 1000
#define SCP_CMD_SCP_BOOT_TIMEOUT_US 5000

int scp_send_cmd(uint32_t cmd, uint32_t param, uint32_t timeout);

#endif

/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURITY_CTRL_PERM_H
#define SECURITY_CTRL_PERM_H

#include "apusys_security_ctrl_perm_plat.h"

enum apusys_sec_level {
	SEC_LEVEL_NORMAL,
	SEC_LEVEL_SECURE,
	SEC_LEVEL_SAPU,
	SEC_LEVEL_AOV,
	SEC_LEVEL_NUM,
};

int sec_set_rv_dns(void);
int sec_get_dns(enum apusys_dev_type dev_type, enum apusys_sec_level sec_level,
		uint8_t *domain, uint8_t *ns);

#endif

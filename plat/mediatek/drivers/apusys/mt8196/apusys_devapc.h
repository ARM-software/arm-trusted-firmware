/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_DEVAPC_H
#define APUSYS_DEVAPC_H

enum apusys_apc_type {
	DAPC_AO = 0,
	DAPC_RCX
};

int apusys_devapc_ao_init(void);
int apusys_devapc_rcx_init(void);

#endif /* APUSYS_DEVAPC_H */

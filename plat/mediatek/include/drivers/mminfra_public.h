/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MMINFRA_PUBLIC_H
#define MMINFRA_PUBLIC_H

#define MMINFRA_RET_ERR		(-1)
#define MMINFRA_RET_POWER_OFF	0
#define MMINFRA_RET_POWER_ON	1

int mminfra_get_if_in_use(void);
int mminfra_put(void);

#endif

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#ifndef SECURE_H
#define SECURE_H

#include <firewall.h>

/* PMU0SGRF */
#define PMU0SGRF_SOC_CON(i)		((i) * 4)

 /* PMU1SGRF */
#define PMU1SGRF_SOC_CON(i)		((i) * 4)

  /* CCISGRF */
#define CCISGRF_SOC_CON(i)		(0x20 + (i) * 4)
#define CCISGRF_DDR_HASH_CON(i)		(0x40 + (i) * 4)

 /* SGRF */
#define SYSSGRF_DDR_BANK_MSK(i)		(0x04 + (i) * 4)
#define SYSSGRF_DDR_CH_MSK(i)		(0x18 + (i) * 4)
#define SYSSGRF_SOC_CON(i)		(0x20 + (i) * 4)
#define SYSSGRF_DMAC_CON(i)		(0x80 + (i) * 4)
#define SYSSGRF_SOC_STATUS		0x240

void secure_init(void);

#endif /* SECURE_H */

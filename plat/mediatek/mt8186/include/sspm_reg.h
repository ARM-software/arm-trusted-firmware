/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SSPM_REG_H__
#define __SSPM_REG_H__

#include "platform_def.h"

#define SSPM_CFGREG_BASE	(IO_PHYS + 0x440000)     /* SSPM view: 0x30040000 */
#define SSPM_CFGREG_ADDR(ofs)	(SSPM_CFGREG_BASE + (ofs))

#define SSPM_MCDI_SHARE_SRAM	(IO_PHYS + 0x420000)
#define SSPM_MBOX_3_BASE	(IO_PHYS + 0x480000)

#define SSPM_HW_SEM		SSPM_CFGREG_ADDR(0x0048)
#define SSPM_ACAO_INT_SET	SSPM_CFGREG_ADDR(0x00D8)
#define SSPM_ACAO_INT_CLR	SSPM_CFGREG_ADDR(0x00DC)

#define STANDBYWFI_EN(n)	(1 << (n + 8))
#define GIC_IRQOUT_EN(n)	(1 << (n + 0))

#endif /* __SSPM_REG_H__ */

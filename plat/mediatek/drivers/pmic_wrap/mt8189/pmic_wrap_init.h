/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_WRAP_INIT_H
#define PMIC_WRAP_INIT_H

#include <platform_def.h>
#include <pmic_wrap_init_common.h>

#define PWRAP_DEBUG				0

/* PMIC_WRAP registers */
struct mt8189_pmic_wrap_regs {
	uint32_t init_done;
	uint32_t reserved[511];
	struct {
		uint32_t cmd;
		uint32_t wdata;
		uint32_t reserved1[3];
		uint32_t rdata;
		uint32_t reserved2[3];
		uint32_t vldclr;
		uint32_t sta;
		uint32_t reserved3[5];
	} wacs[4];
};

static struct mt8189_pmic_wrap_regs *const mtk_pwrap = (void *)PMIC_WRAP_BASE;

#define PMIF_SPI_SWINF_NO			2

#endif /* PMIC_WRAP_INIT_H */

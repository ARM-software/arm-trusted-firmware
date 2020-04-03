/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SWREG_H
#define SWREG_H

/* default voltage if no valid OTP */
#define VDDC_CORE_DEF_VOLT      910000  /* 0.91v */
#define IHOST_DEF_VOLT          940000  /* 0.94v */

#define B0_VDDC_CORE_DEF_VOLT   950000  /* 0.95v */
#define B0_IHOST_DEF_VOLT       950000  /* 0.95v */
#define B0_DDR_VDDC_DEF_VOLT    1000000 /* 1v */

#define SWREG_IHOST1_DIS        4
#define SWREG_IHOST1_REG_RESETB 5
#define SWREG_IHOST1_PMU_STABLE 2

enum sw_reg {
	DDR_VDDC = 1,
	IHOST03,
	IHOST12,
	IHOST_ARRAY,
	DDRIO_SLAVE,
	VDDC_CORE,
	VDDC1,
	DDRIO_MASTER
};

int set_swreg(enum sw_reg reg_id, uint32_t micro_volts);
int swreg_firmware_update(void);

#endif

/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_PMIC_LP_H
#define MT_SPM_PMIC_LP_H

enum SPM_PWR_TYPE {
	SPM_LP_ENTER,
	SPM_LP_RESUME
};

enum {
	LP_MT6363 = 0,
	LP_MT6373,
	LP_MT6316_1,
	LP_MT6316_2,
	LP_MT6316_3,
	LP_MT6316_4,
	LP_PMIC_SLAVE_NUM,
};

#ifdef MTK_SPM_PMIC_LP_SUPPORT
void set_vcore_lp_enable(bool enable);
bool get_vcore_lp_enable(void);

void set_vsram_lp_enable(bool enable);
bool get_vsram_lp_enable(void);
void set_vsram_lp_volt(uint32_t volt);
uint32_t get_vsram_lp_volt(void);

int do_spm_low_power(enum SPM_PWR_TYPE type, uint32_t cmd);
#else
static inline void set_vcore_lp_enable(bool enable)
{
	(void)enable;
}

static inline bool get_vcore_lp_enable(void)
{
	return false;
}

static inline void set_vsram_lp_enable(bool enable)
{
	(void)enable;
}

static inline bool get_vsram_lp_enable(void)
{
	return false;
}

static inline void set_vsram_lp_volt(uint32_t volt)
{
	(void)volt;
}

static inline uint32_t get_vsram_lp_volt(void)
{
	return 0;
}

static inline int do_spm_low_power(enum SPM_PWR_TYPE type, uint32_t cmd)
{
	(void)type;
	(void)cmd;
	return 0;
}
#endif /* MTK_SPM_PMIC_LP_SUPPORT */

#endif /* MT_SPM_PMIC_LP_H */

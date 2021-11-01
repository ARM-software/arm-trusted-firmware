/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AXP_H
#define AXP_H

#include <stdint.h>

#define AXP20X_MODE_REG 0x3e
#define AXP20X_MODE_I2C 0x00
#define AXP20X_MODE_RSB 0x7c

#define NA 0xff

enum {
	AXP803_CHIP_ID = 0x41,
	AXP805_CHIP_ID = 0x40,
};

struct axp_regulator {
	const char *dt_name;
	uint16_t min_volt;
	uint16_t max_volt;
	uint16_t step;
	unsigned char split;
	unsigned char volt_reg;
	unsigned char switch_reg;
	unsigned char switch_bit;
};

extern const uint8_t axp_chip_id;
extern const char *const axp_compatible;
extern const struct axp_regulator axp_regulators[];

/*
 * Since the PMIC can be connected to multiple bus types,
 * low-level read/write functions must be provided by the platform
 */
int axp_read(uint8_t reg);
int axp_write(uint8_t reg, uint8_t val);
int axp_clrsetbits(uint8_t reg, uint8_t clr_mask, uint8_t set_mask);
#define axp_clrbits(reg, clr_mask) axp_clrsetbits(reg, clr_mask, 0)
#define axp_setbits(reg, set_mask) axp_clrsetbits(reg, 0, set_mask)

int axp_check_id(void);
void axp_power_off(void);

#if SUNXI_SETUP_REGULATORS == 1
void axp_setup_regulators(const void *fdt);
#else
static inline void axp_setup_regulators(const void *fdt)
{
}
#endif

#endif /* AXP_H */

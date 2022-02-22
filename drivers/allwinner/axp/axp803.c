/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/allwinner/axp.h>

const uint8_t axp_chip_id = AXP803_CHIP_ID;
const char *const axp_compatible = "x-powers,axp803";

#if SUNXI_SETUP_REGULATORS == 1
const struct axp_regulator axp_regulators[] = {
	{"aldo1",  700, 3300, 100, NA, 0x28, 0x13, 5},
	{"dcdc1", 1600, 3400, 100, NA, 0x20, 0x10, 0},
	{"dcdc5",  800, 1840,  10, 32, 0x24, 0x10, 4},
	{"dcdc6",  600, 1520,  10, 50, 0x25, 0x10, 5},
	{"dldo1",  700, 3300, 100, NA, 0x15, 0x12, 3},
	{"dldo2",  700, 4200, 100, 27, 0x16, 0x12, 4},
	{"dldo3",  700, 3300, 100, NA, 0x17, 0x12, 5},
	{"dldo4",  700, 3300, 100, NA, 0x18, 0x12, 6},
	{"fldo1",  700, 1450,  50, NA, 0x1c, 0x13, 2},
	{}
};
#endif

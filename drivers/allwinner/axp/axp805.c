/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/allwinner/axp.h>

const uint8_t axp_chip_id = AXP805_CHIP_ID;
const char *const axp_compatible = "x-powers,axp805";

/*
 * The "dcdcd" split changes the step size by a factor of 5, not 2;
 * disallow values above the split to maintain accuracy.
 */
const struct axp_regulator axp_regulators[] = {
	{"dcdca",  600, 1520,  10, 50, 0x12, 0x10, 0},
	{"dcdcb", 1000, 2550,  50, NA, 0x13, 0x10, 1},
	{"dcdcc",  600, 1520,  10, 50, 0x14, 0x10, 2},
	{"dcdcd",  600, 1500,  20, NA, 0x15, 0x10, 3},
	{"dcdce", 1100, 3400, 100, NA, 0x16, 0x10, 4},
	{"aldo1",  700, 3300, 100, NA, 0x17, 0x10, 5},
	{"aldo2",  700, 3300, 100, NA, 0x18, 0x10, 6},
	{"aldo3",  700, 3300, 100, NA, 0x19, 0x10, 7},
	{"bldo1",  700, 1900, 100, NA, 0x20, 0x11, 0},
	{"bldo2",  700, 1900, 100, NA, 0x21, 0x11, 1},
	{"bldo3",  700, 1900, 100, NA, 0x22, 0x11, 2},
	{"bldo4",  700, 1900, 100, NA, 0x23, 0x11, 3},
	{"cldo1",  700, 3300, 100, NA, 0x24, 0x11, 4},
	{"cldo2",  700, 4200, 100, 27, 0x25, 0x11, 5},
	{"cldo3",  700, 3300, 100, NA, 0x26, 0x11, 6},
	{}
};

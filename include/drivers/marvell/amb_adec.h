/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* AXI to M-Bridge decoding unit driver for Marvell Armada 8K and 8K+ SoCs */

#ifndef AMB_ADEC_H
#define AMB_ADEC_H

#include <stdint.h>

enum amb_attribute_ids {
	AMB_SPI0_CS0_ID = 0x1E,
	AMB_SPI0_CS1_ID = 0x5E,
	AMB_SPI0_CS2_ID = 0x9E,
	AMB_SPI0_CS3_ID = 0xDE,
	AMB_SPI1_CS0_ID = 0x1A,
	AMB_SPI1_CS1_ID = 0x5A,
	AMB_SPI1_CS2_ID = 0x9A,
	AMB_SPI1_CS3_ID = 0xDA,
	AMB_DEV_CS0_ID = 0x3E,
	AMB_DEV_CS1_ID = 0x3D,
	AMB_DEV_CS2_ID = 0x3B,
	AMB_DEV_CS3_ID = 0x37,
	AMB_BOOT_CS_ID = 0x2f,
	AMB_BOOT_ROM_ID = 0x1D,
};

#define AMB_MAX_WIN_ID		7

int init_amb_adec(uintptr_t base);

#endif /* AMB_ADEC_H */

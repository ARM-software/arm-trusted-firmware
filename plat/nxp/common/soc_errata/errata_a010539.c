/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <mmio.h>

#include <plat_common.h>

void erratum_a010539(void)
{
	if (get_boot_dev() == BOOT_DEVICE_QSPI) {
		unsigned int *porsr1 = (void *)(NXP_DCFG_ADDR +
				DCFG_PORSR1_OFFSET);
		uint32_t val;

		val = (gur_in32(porsr1) & ~PORSR1_RCW_MASK);
		mmio_write_32((uint32_t)(NXP_DCSR_DCFG_ADDR +
				DCFG_DCSR_PORCR1_OFFSET), htobe32(val));
		/* Erratum need to set '1' to all bits for reserved SCFG register 0x1a8 */
		mmio_write_32((uint32_t)(NXP_SCFG_ADDR + 0x1a8),
				htobe32(0xffffffff));
	}
}

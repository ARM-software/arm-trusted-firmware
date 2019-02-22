/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#if ERRATA_LS_A008850
#include <drivers/arm/cci.h>
#endif
#include <lib/mmio.h>

#include <io.h>
#include <plat_common.h>

void erratum_a008850_early(void)
{
#if ERRATA_LS_A008850
	/* part 1 of 2 */
	uintptr_t cci_base = NXP_CCI_ADDR;
	uint32_t val = mmio_read_32(cci_base + CTRL_OVERRIDE_REG);

	/* enabling forced barrier termination on CCI400 */
	mmio_write_32(cci_base + CTRL_OVERRIDE_REG,
		      (val | CCI_TERMINATE_BARRIER_TX));

#endif
}

void erratum_a008850_post(void)
{
#if ERRATA_LS_A008850
	/* part 2 of 2 */
	uintptr_t cci_base = NXP_CCI_ADDR;
	uint32_t val = mmio_read_32(cci_base + CTRL_OVERRIDE_REG);

	/* Clear the BARRIER_TX bit */
	val = val & ~(CCI_TERMINATE_BARRIER_TX);

	/*
	 * Disable barrier termination on CCI400, allowing
	 * barriers to propagate across CCI
	 */
	mmio_write_32(cci_base + CTRL_OVERRIDE_REG, val);

#endif
}

void erratum_a009660(void)
{
#if ERRATA_LS_A009660
	scfg_setbits32((NXP_SCFG_ADDR + 0x20c),
			0x63b20042);
#endif
}

void erratum_a010539(void)
{
#if ERRATA_LS_A010539
	if (get_boot_dev() == BOOT_DEVICE_QSPI) {
		uint32_t val;

		val = (gur_in32((NXP_DCFG_ADDR + DCFG_PORSR1_OFFSET)) &
							~PORSR1_RCW_MASK);
		out_be32((NXP_DCSR_DCFG_ADDR + DCFG_DCSR_PORCR1_OFFSET),
				  val);
		out_be32((NXP_SCFG_ADDR + 0x1a8), 0xffffffff);
	}

#endif
}


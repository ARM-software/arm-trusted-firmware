/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <cci.h>
#include <common/debug.h>
#include <ls_interconnect.h>
#include <mmio.h>

#include <platform_def.h>

void erratum_a008850_early(void)
{
	/* part 1 of 2 */
	uintptr_t cci_base = NXP_CCI_ADDR;
	uint32_t val = mmio_read_32(cci_base + CTRL_OVERRIDE_REG);

	/* enabling forced barrier termination on CCI400 */
	mmio_write_32(cci_base + CTRL_OVERRIDE_REG,
		      (val | CCI_TERMINATE_BARRIER_TX));

}

void erratum_a008850_post(void)
{
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

	INFO("SoC workaround for Errata A008850 Post-Phase was applied\n");
}

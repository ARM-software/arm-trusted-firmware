/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Zynq UltraScale+ MPSoC IPI agent registers access management
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include <ipi.h>
#include <plat_ipi.h>
#include <plat_private.h>

/* Zynqmp ipi configuration table */
static const struct ipi_config zynqmp_ipi_table[] = {
	/* APU IPI */
	{
		.ipi_bit_mask = 0x1,
		.ipi_reg_base = 0xFF300000U,
		.secure_only = 0,
	},
	/* RPU0 IPI */
	{
		.ipi_bit_mask = 0x100,
		.ipi_reg_base = 0xFF310000U,
		.secure_only = 0,
	},
	/* RPU1 IPI */
	{
		.ipi_bit_mask = 0x200,
		.ipi_reg_base = 0xFF320000U,
		.secure_only = 0,
	},
	/* PMU0 IPI */
	{
		.ipi_bit_mask = 0x10000,
		.ipi_reg_base = 0xFF330000U,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PMU1 IPI */
	{
		.ipi_bit_mask = 0x20000,
		.ipi_reg_base = 0xFF331000U,
		.secure_only = 0,
	},
	/* PMU2 IPI */
	{
		.ipi_bit_mask = 0x40000,
		.ipi_reg_base = 0xFF332000U,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PMU3 IPI */
	{
		.ipi_bit_mask = 0x80000,
		.ipi_reg_base = 0xFF333000U,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PL0 IPI */
	{
		.ipi_bit_mask = 0x1000000,
		.ipi_reg_base = 0xFF340000U,
		.secure_only = 0,
	},
	/* PL1 IPI */
	{
		.ipi_bit_mask = 0x2000000,
		.ipi_reg_base = 0xFF350000U,
		.secure_only = 0,
	},
	/* PL2 IPI */
	{
		.ipi_bit_mask = 0x4000000,
		.ipi_reg_base = 0xFF360000U,
		.secure_only = 0,
	},
	/* PL3 IPI */
	{
		.ipi_bit_mask = 0x8000000,
		.ipi_reg_base = 0xFF370000U,
		.secure_only = 0,
	},
};

/**
 * zynqmp_ipi_config_table_init() - Initialize ZynqMP IPI configuration data
 *
 */
void zynqmp_ipi_config_table_init(void)
{
	ipi_config_table_init(zynqmp_ipi_table, ARRAY_SIZE(zynqmp_ipi_table));
}

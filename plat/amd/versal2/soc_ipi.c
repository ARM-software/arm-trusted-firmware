/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * SoC IPI agent registers access management
 */

#include <lib/utils_def.h>
#include <plat_ipi.h>

/* versal2 ipi configuration table */
static const struct ipi_config ipi_table[IPI_ID_MAX] = {
	/* A78 IPI */
	[IPI_ID_APU] = {
		.ipi_bit_mask = IPI0_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = 0,
	},

	/* PMC IPI */
	[IPI_ID_PMC] = {
		.ipi_bit_mask = PMC_IPI_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = IPI_SECURE_MASK,
	},

	/* RPU0 IPI */
	[IPI_ID_RPU0] = {
		.ipi_bit_mask = IPI1_TRIG_BIT,
		.ipi_reg_base = IPI1_REG_BASE,
		.secure_only = 0,
	},

	/* RPU1 IPI */
	[IPI_ID_RPU1] = {
		.ipi_bit_mask = IPI2_TRIG_BIT,
		.ipi_reg_base = IPI2_REG_BASE,
		.secure_only = 0,
	},

	/* IPI3 IPI */
	[IPI_ID_3] = {
		.ipi_bit_mask = IPI3_TRIG_BIT,
		.ipi_reg_base = IPI3_REG_BASE,
		.secure_only = 0,
	},

	/* IPI4 IPI */
	[IPI_ID_4] = {
		.ipi_bit_mask = IPI4_TRIG_BIT,
		.ipi_reg_base = IPI4_REG_BASE,
		.secure_only = 0,
	},

	/* IPI5 IPI */
	[IPI_ID_5] = {
		.ipi_bit_mask = IPI5_TRIG_BIT,
		.ipi_reg_base = IPI5_REG_BASE,
		.secure_only = 0,
	},
};

/**
 * soc_ipi_config_table_init() - Initialize versal2 IPI configuration data.
 */
void soc_ipi_config_table_init(void)
{
	ipi_config_table_init(ipi_table, ARRAY_SIZE(ipi_table));
}

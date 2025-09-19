/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * SoC IPI agent registers access management
 */

#include <plat_ipi.h>

/* Versal Gen 2 ipi configuration table */
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

	/* PMC_NOBUF IPI */
	[IPI_ID_PMC_NOBUF] = {
		.ipi_bit_mask = PMC_NOBUF_TRIG_BIT,
		.ipi_reg_base = PMC_NOBUF_REG_BASE,
		.secure_only = IPI_SECURE_MASK,
	},

	/* IPI6 IPI */
	[IPI_ID_6_NOBUF_95] = {
		.ipi_bit_mask = IPI6_NOBUF_95_TRIG_BIT,
		.ipi_reg_base = IPI6_NOBUF_95_REG_BASE,
		.secure_only = 0,
	},

	/* IPI1 NO BUF IPI */
	[IPI_ID_1_NOBUF] = {
		.ipi_bit_mask = IPI1_NOBUF_TRIG_BIT,
		.ipi_reg_base = IPI1_NOBUF_REG_BASE,
		.secure_only = 0,
	},

	/* IPI2 NO BUF IPI */
	[IPI_ID_2_NOBUF] = {
		.ipi_bit_mask = IPI2_NOBUF_TRIG_BIT,
		.ipi_reg_base = IPI2_NOBUF_REG_BASE,
		.secure_only = 0,
	},

	/* IPI3 NO BUF IPI */
	[IPI_ID_3_NOBUF] = {
		.ipi_bit_mask = IPI3_NOBUF_TRIG_BIT,
		.ipi_reg_base = IPI3_NOBUF_REG_BASE,
		.secure_only = 0,
	},

	/* IPI4 NO BUF IPI */
	[IPI_ID_4_NOBUF] = {
		.ipi_bit_mask = IPI4_NOBUF_TRIG_BIT,
		.ipi_reg_base = IPI4_NOBUF_REG_BASE,
		.secure_only = 0,
	},

	/* IPI5 NO BUF IPI */
	[IPI_ID_5_NOBUF] = {
		.ipi_bit_mask = IPI5_NOBUF_TRIG_BIT,
		.ipi_reg_base = IPI5_NOBUF_REG_BASE,
		.secure_only = 0,
	},

	/* IPI6 NO BUF IPI */
	[IPI_ID_6_NOBUF_101] = {
		.ipi_bit_mask = IPI6_NOBUF_101_TRIG_BIT,
		.ipi_reg_base = IPI6_NOBUF_101_REG_BASE,
		.secure_only = 0,
	},
};

/**
 * soc_ipi_config_table_init() - Initialize Versal Gen 2 IPI configuration data.
 */
void soc_ipi_config_table_init(void)
{
	ipi_config_table_init(ipi_table, ARRAY_SIZE(ipi_table));
}

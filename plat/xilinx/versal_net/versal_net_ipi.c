/*
 * Copyright (C) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal NET IPI agent registers access management
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

/* versal_net ipi configuration table */
static const struct ipi_config versal_net_ipi_table[IPI_ID_MAX] = {
	/* A72 IPI */
	[IPI_ID_APU] = {
		.ipi_bit_mask = IPI0_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = 0,
	},

	/* PMC IPI */
	[IPI_ID_PMC] = {
		.ipi_bit_mask = PMC_IPI_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = 0,
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

/* versal_net_ipi_config_table_init() - Initialize versal_net IPI configuration data
 *
 * @ipi_config_table  - IPI configuration table
 * @ipi_total - Total number of IPI available
 *
 */
void versal_net_ipi_config_table_init(void)
{
	ipi_config_table_init(versal_net_ipi_table, ARRAY_SIZE(versal_net_ipi_table));
}

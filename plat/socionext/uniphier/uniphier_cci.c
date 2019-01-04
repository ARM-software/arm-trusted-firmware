/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <arch_helpers.h>
#include <drivers/arm/cci.h>
#include <lib/utils_def.h>

#include "uniphier.h"

#define UNIPHIER_CCI500_BASE	0x5FD00000

static const int uniphier_cci_map[] = {1, 0};

static void __uniphier_cci_init(void)
{
	cci_init(UNIPHIER_CCI500_BASE, uniphier_cci_map,
		 ARRAY_SIZE(uniphier_cci_map));
}

static void __uniphier_cci_enable(void)
{
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

static void __uniphier_cci_disable(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

struct uniphier_cci_ops {
	void (*init)(void);
	void (*enable)(void);
	void (*disable)(void);
};

static const struct uniphier_cci_ops uniphier_cci_ops_table[] = {
	[UNIPHIER_SOC_LD11] = {
		.init = NULL,
		.enable = NULL,
		.disable = NULL,
	},
	[UNIPHIER_SOC_LD20] = {
		.init = __uniphier_cci_init,
		.enable = __uniphier_cci_enable,
		.disable = __uniphier_cci_disable,
	},
	[UNIPHIER_SOC_PXS3] = {
		.init = NULL,
		.enable = NULL,
		.disable = NULL,
	},
};

static struct uniphier_cci_ops uniphier_cci_ops;

void uniphier_cci_init(unsigned int soc)
{
	uniphier_cci_ops = uniphier_cci_ops_table[soc];
	flush_dcache_range((uint64_t)&uniphier_cci_ops,
			   sizeof(uniphier_cci_ops));

	if (uniphier_cci_ops.init)
		uniphier_cci_ops.init();
}

void uniphier_cci_enable(void)
{
	if (uniphier_cci_ops.enable)
		uniphier_cci_ops.enable();
}

void uniphier_cci_disable(void)
{
	if (uniphier_cci_ops.disable)
		uniphier_cci_ops.disable();
}

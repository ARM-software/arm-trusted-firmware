/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>

#include <drivers/clk.h>

static int s32cc_clk_enable(unsigned long id)
{
	return -ENOTSUP;
}

static void s32cc_clk_disable(unsigned long id)
{
}

static bool s32cc_clk_is_enabled(unsigned long id)
{
	return false;
}

static unsigned long s32cc_clk_get_rate(unsigned long id)
{
	return 0;
}

static int s32cc_clk_set_rate(unsigned long id, unsigned long rate,
			      unsigned long *orate)
{
	return -ENOTSUP;
}

static int s32cc_clk_get_parent(unsigned long id)
{
	return -ENOTSUP;
}

static int s32cc_clk_set_parent(unsigned long id, unsigned long parent_id)
{
	return -ENOTSUP;
}

void s32cc_clk_register_drv(void)
{
	static const struct clk_ops s32cc_clk_ops = {
		.enable		= s32cc_clk_enable,
		.disable	= s32cc_clk_disable,
		.is_enabled	= s32cc_clk_is_enabled,
		.get_rate	= s32cc_clk_get_rate,
		.set_rate	= s32cc_clk_set_rate,
		.get_parent	= s32cc_clk_get_parent,
		.set_parent	= s32cc_clk_set_parent,
	};

	clk_register(&s32cc_clk_ops);
}


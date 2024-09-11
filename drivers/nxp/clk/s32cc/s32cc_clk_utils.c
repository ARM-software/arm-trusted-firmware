/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <s32cc-clk-ids.h>
#include <s32cc-clk-utils.h>

static struct s32cc_clk *s32cc_clk_get_from_array(const struct s32cc_clk_array *arr,
						  unsigned long clk_id)
{
	unsigned long type, id;

	type = S32CC_CLK_TYPE(clk_id);

	if (type != arr->type_mask) {
		return NULL;
	}

	id = S32CC_CLK_ID(clk_id);

	if (id >= arr->n_clks) {
		return NULL;
	}

	return arr->clks[id];
}

struct s32cc_clk *s32cc_get_clk_from_table(const struct s32cc_clk_array *const *clk_arr,
					   size_t size,
					   unsigned long clk_id)
{
	struct s32cc_clk *clk;
	size_t i;

	for (i = 0; i < size; i++) {
		clk = s32cc_clk_get_from_array(clk_arr[i], clk_id);
		if (clk != NULL) {
			return clk;
		}
	}

	return NULL;
}

int s32cc_get_id_from_table(const struct s32cc_clk_array *const *clk_arr,
			    size_t size, const struct s32cc_clk *clk,
			    unsigned long *clk_index)
{
	size_t i, j;

	for (i = 0; i < size; i++) {
		for (j = 0; j < clk_arr[i]->n_clks; j++) {
			if (clk_arr[i]->clks[j] != clk) {
				continue;
			}

			*clk_index = S32CC_CLK(clk_arr[i]->type_mask, j);
			return 0;
		}
	}

	return -EINVAL;
}

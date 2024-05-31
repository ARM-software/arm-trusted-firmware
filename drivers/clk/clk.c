/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 * Author(s): Ludovic Barre, <ludovic.barre@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <drivers/clk.h>

static const struct clk_ops *ops;

int clk_enable(unsigned long id)
{
	assert((ops != NULL) && (ops->enable != NULL));

	return ops->enable(id);
}

void clk_disable(unsigned long id)
{
	assert((ops != NULL) && (ops->disable != NULL));

	ops->disable(id);
}

unsigned long clk_get_rate(unsigned long id)
{
	assert((ops != NULL) && (ops->get_rate != NULL));

	return ops->get_rate(id);
}

int clk_set_rate(unsigned long id, unsigned long rate, unsigned long *orate)
{
	unsigned long lrate;

	assert((ops != NULL) && (ops->set_rate != NULL));

	if (orate != NULL) {
		return ops->set_rate(id, rate, orate);
	}

	/* In case the caller is not interested in the output rate */
	return ops->set_rate(id, rate, &lrate);
}

int clk_get_parent(unsigned long id)
{
	assert((ops != NULL) && (ops->get_parent != NULL));

	return ops->get_parent(id);
}

int clk_set_parent(unsigned long id, unsigned long parent_id)
{
	assert((ops != NULL) && (ops->set_parent != NULL));

	return ops->set_parent(id, parent_id);
}

bool clk_is_enabled(unsigned long id)
{
	assert((ops != NULL) && (ops->is_enabled != NULL));

	return ops->is_enabled(id);
}

/*
 * Initialize the clk. The fields in the provided clk
 * ops pointer must be valid.
 */
void clk_register(const struct clk_ops *ops_ptr)
{
	assert((ops_ptr != NULL) &&
	       (ops_ptr->enable != NULL) &&
	       (ops_ptr->disable != NULL) &&
	       (ops_ptr->get_rate != NULL) &&
	       (ops_ptr->get_parent != NULL) &&
	       (ops_ptr->is_enabled != NULL));

	ops = ops_ptr;
}

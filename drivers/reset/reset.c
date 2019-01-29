/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/reset.h>

static const struct reset_ops *reset_ops;

void reset_assert(unsigned int id)
{
	assert((reset_ops != NULL) && (reset_ops->reset_assert != NULL));

	reset_ops->reset_assert(id);
}

void reset_deassert(unsigned int id)
{
	assert((reset_ops != NULL) && (reset_ops->reset_deassert != NULL));

	reset_ops->reset_deassert(id);
}

bool reset_asserted_status(unsigned int id)
{
	assert((reset_ops != NULL) &&
	       (reset_ops->reset_asserted_status != NULL));

	return reset_ops->reset_asserted_status(id);
}

void register_reset_driver(const struct reset_ops *ops)
{
	assert((ops != NULL) && (reset_ops == NULL));

	reset_ops = ops;
}

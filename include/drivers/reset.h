/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RESET_H
#define RESET_H

#include <stdbool.h>

struct reset_ops {
	void (*reset_assert)(unsigned int id);
	void (*reset_deassert)(unsigned int id);
	/* reset_asserted_status: true = asserted, false = deasserted */
	bool (*reset_asserted_status)(unsigned int id);
};

void reset_assert(unsigned int id);
void reset_deassert(unsigned int id);
bool reset_asserted_status(unsigned int id);
void register_reset_driver(const struct reset_ops *ops);

#endif /* RESET_H */

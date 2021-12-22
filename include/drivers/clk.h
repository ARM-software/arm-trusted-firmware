/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_H
#define CLK_H

#include <stdbool.h>

struct clk_ops {
	int (*enable)(unsigned long id);
	void (*disable)(unsigned long id);
	unsigned long (*get_rate)(unsigned long id);
	int (*get_parent)(unsigned long id);
	bool (*is_enabled)(unsigned long id);
};

int clk_enable(unsigned long id);
void clk_disable(unsigned long id);
unsigned long clk_get_rate(unsigned long id);
bool clk_is_enabled(unsigned long id);
int clk_get_parent(unsigned long id);

void clk_register(const struct clk_ops *ops);

#endif /* CLK_H */

/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

typedef struct dev dev_t;

extern dev_t rootdevtab;

dev_t *const devtab[] = {
	&rootdevtab,
	0
};

void devlink(void)
{
}

/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

typedef struct dev dev_t;

extern dev_t rootdevtab;
extern dev_t fipdevtab;

dev_t *const devtab[] = {
	&rootdevtab,
	&fipdevtab,
	0
};

void devlink(void)
{
}

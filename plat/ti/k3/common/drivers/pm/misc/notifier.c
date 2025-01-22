/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <notifier.h>

void notifier_call(struct list_head *head, void *data)
{
	struct notifier *n;
	struct notifier *next;

	/* *INDENT-OFF* */
	list_for_each_safe(head, n, next, node) {
		n->fn(n, data);
		/* *INDENT-ON* */
	}
}

/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <list.h>

struct notifier {
	void			(*fn)(struct notifier *n, void *data);
	struct list_node	node;
};

void notifier_call(struct list_head *head, void *data);

static inline void notifier_register(struct list_head	*head,
				     struct notifier	*n)
{
	list_add(head, &n->node);
}

static inline void notifier_unregister(struct notifier *n)
{
	list_del(&n->node);
}

#endif

/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RQ_H
#define MT_LP_RQ_H

/* Determine the generic resource request public type */
#define MT_LP_RQ_XO_FPM		BIT(0)
#define MT_LP_RQ_26M		BIT(1)
#define MT_LP_RQ_INFRA		BIT(2)
#define MT_LP_RQ_SYSPLL		BIT(3)
#define MT_LP_RQ_DRAM		BIT(4)
#define MT_LP_RQ_ALL		(0xFFFFFFFF)

struct mt_lp_resource_user {
	/* Determine the resource user mask */
	unsigned int umask;
	/* Determine the resource request user identify */
	unsigned int uid;
	/* Request the resource */
	int (*request)(struct mt_lp_resource_user *this, unsigned int resource);
	/* Release the resource */
	int (*release)(struct mt_lp_resource_user *this);
};

int mt_lp_resource_user_register(char *uname, struct mt_lp_resource_user *ru);

#endif /* MT_LP_RQ_H */

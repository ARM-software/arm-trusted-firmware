/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RQM_H
#define MT_LP_RQM_H

#include "mt_lp_rq.h"

enum plat_mt_lpm_rq_update_type {
	PLAT_RQ_USER_NUM,
	PLAT_RQ_USER_VALID,
	PLAT_RQ_USER_REQ,
	PLAT_RQ_USER_REL,
	PLAT_RQ_PER_USER_NAME,
	PLAT_RQ_REQ_NUM,
	PLAT_RQ_REQ_USAGE,
};

/* Determine the request valid */
#define MT_LP_RQ_VALID		(0x1)
#define MT_LP_RQ_INVALID	(0x0)

/* Determine the request user opertions */
#define MT_LP_RQ_USER_INVALID	(-1)
#define MT_LP_RQ_USER_MAX	(32)
#define MT_LP_RQ_USER_NAME_LEN	(4)
#define MT_LP_RQ_USER_CHAR_U	(8)

/* Determine the request update flag */
#define MT_LP_RQ_FLAG_DONE		(0)
#define MT_LP_RQ_FLAG_NEED_UPDATE	BIT(6)

/* Determine the resource update id */
#define MT_LP_RQ_ID_ALL_USAGE	(-1)

/* Determine the return status */
#define MT_LP_RQ_STA_OK		(0)
#define MT_LP_RQ_STA_BAD	(-1)

struct mt_lp_res_req {
	/* Determine the resource req public identify */
	const unsigned int res_id;
	/* Determine the resource bitwise internal control */
	const unsigned int res_rq;
	/* Determine the users per bit for current resource usage */
	unsigned int res_usage;
};

struct mt_resource_req_manager {
	/* Determine the set of resources */
	struct mt_lp_res_req **res;
};

struct resource_req_status {
	/* Determine the status id */
	unsigned int id;
	/* Determine the status value */
	unsigned int val;
};

int mt_lp_resource_request_manager_register(struct mt_resource_req_manager *rqm);
int mt_lp_rq_update_status(int type, void *p);
int mt_lp_rq_get_status(int type, void *p);

#endif /* MT_LP_RQM_H */

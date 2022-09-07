/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/spinlock.h>
#include <lpm/mt_lp_rqm.h>

struct mt_lp_res_req_m {
	unsigned int uname[MT_LP_RQ_USER_MAX];
	unsigned int user_num;
	unsigned int user_valid;
	unsigned int resource_num;
	unsigned int generic_resource_req;
	unsigned int flag;
	struct mt_resource_req_manager *plat_rqm;
};

static struct mt_lp_res_req_m plat_mt_rqm;
static spinlock_t mt_lp_rq_lock;

static int mt_lp_resource_request(struct mt_lp_resource_user *this, unsigned int resource)
{
	int i;
	struct mt_lp_res_req *const *rs;

	if ((this == NULL) || (resource == 0) || (resource > MT_LP_RQ_ALL)) {
		ERROR("invalid request(%x)\n", resource);
		return MT_LP_RQ_STA_BAD;
	}

	spin_lock(&mt_lp_rq_lock);

	rs = (plat_mt_rqm.plat_rqm)->res;
	for (i = 0; i < plat_mt_rqm.resource_num; i++) {
		if ((resource & rs[i]->res_id) != 0) {
			rs[i]->res_usage |= this->umask;
		}
	}

	plat_mt_rqm.flag = MT_LP_RQ_FLAG_NEED_UPDATE;
	spin_unlock(&mt_lp_rq_lock);

	return MT_LP_RQ_STA_OK;
}

static int mt_lp_resource_release(struct mt_lp_resource_user *this)
{
	int i;
	struct mt_lp_res_req *const *rs;

	if (this == NULL) {
		return MT_LP_RQ_STA_BAD;
	}

	spin_lock(&mt_lp_rq_lock);

	rs = (plat_mt_rqm.plat_rqm)->res;
	for (i = 0; i < plat_mt_rqm.resource_num; i++) {
		rs[i]->res_usage &= ~(this->umask);
	}

	plat_mt_rqm.flag = MT_LP_RQ_FLAG_NEED_UPDATE;
	spin_unlock(&mt_lp_rq_lock);

	return MT_LP_RQ_STA_OK;
}

int mt_lp_resource_request_manager_register(struct mt_resource_req_manager *rqm)
{
	unsigned int count;
	struct mt_lp_res_req *const *rs;

	if ((rqm == NULL) || (rqm->res == NULL) || (plat_mt_rqm.plat_rqm != NULL)) {
		return MT_LP_RQ_STA_BAD;
	}

	rs = rqm->res;
	count = 0;
	while (*rs != NULL) {
		count++;
		rs++;
	}

	plat_mt_rqm.plat_rqm = rqm;
	plat_mt_rqm.resource_num = count;

	return MT_LP_RQ_STA_OK;
}

int mt_lp_resource_user_register(char *user, struct mt_lp_resource_user *ru)
{
	int i, len;
	unsigned int uname;

	if ((plat_mt_rqm.plat_rqm == NULL) || (plat_mt_rqm.user_num >= MT_LP_RQ_USER_MAX) ||
	    (user == NULL)) {
		ru->uid = MT_LP_RQ_USER_INVALID;
		ru->umask = 0;
		ru->request = NULL;
		ru->release = NULL;
		ERROR("rqm register user invalid\n");
		return MT_LP_RQ_STA_BAD;
	}

	len = strnlen(user, MT_LP_RQ_USER_NAME_LEN);

	uname = 0;
	for (i = 0; i < len; i++) {
		uname |= (user[i] << (MT_LP_RQ_USER_CHAR_U * i));
	}

	spin_lock(&mt_lp_rq_lock);
	i = plat_mt_rqm.user_num;
	plat_mt_rqm.user_num += 1;
	plat_mt_rqm.uname[i] = uname;
	plat_mt_rqm.user_valid |= BIT(i);
	spin_unlock(&mt_lp_rq_lock);

	ru->umask = BIT(i);
	ru->uid = i;
	ru->request = mt_lp_resource_request;
	ru->release = mt_lp_resource_release;
	INFO("%s register by %s, uid = %d\n", __func__, user, ru->uid);

	return MT_LP_RQ_STA_OK;
}

int mt_lp_rq_get_status(int type, void *p)
{
	int i;
	unsigned int update_sta;
	struct mt_lp_res_req *const *rs;
	struct resource_req_status *rq_sta = (struct resource_req_status *)p;

	if (plat_mt_rqm.flag != 0) {
		spin_lock(&mt_lp_rq_lock);

		update_sta = 0;
		rs = (plat_mt_rqm.plat_rqm)->res;
		for (i = 0; i < plat_mt_rqm.resource_num; i++) {
			update_sta |= ((rs[i]->res_usage & plat_mt_rqm.user_valid) != 0) ?
				      rs[i]->res_rq : 0;
		}

		plat_mt_rqm.generic_resource_req = update_sta;
		plat_mt_rqm.flag = MT_LP_RQ_FLAG_DONE;
		spin_unlock(&mt_lp_rq_lock);
	}

	switch (type) {
	case PLAT_RQ_REQ_USAGE:
		rs = (plat_mt_rqm.plat_rqm)->res;
		rq_sta->val = (rq_sta->id < plat_mt_rqm.resource_num) ?
			      rs[rq_sta->id]->res_usage : plat_mt_rqm.generic_resource_req;
		break;
	case PLAT_RQ_USER_NUM:
		rq_sta->val = plat_mt_rqm.user_num;
		break;
	case PLAT_RQ_USER_VALID:
		rq_sta->val = plat_mt_rqm.user_valid;
		break;
	case PLAT_RQ_PER_USER_NAME:
		rq_sta->val = (rq_sta->id < plat_mt_rqm.user_num) ?
			      plat_mt_rqm.uname[rq_sta->id] : 0;
		break;
	case PLAT_RQ_REQ_NUM:
		rq_sta->val = plat_mt_rqm.resource_num;
		break;
	default:
		break;
	}

	return MT_LP_RQ_STA_OK;
}

int mt_lp_rq_update_status(int type, void *p)
{
	unsigned int user_mask;
	struct resource_req_status *rq_sta = (struct resource_req_status *)p;

	switch (type) {
	case PLAT_RQ_USER_VALID:
		if (rq_sta->id < plat_mt_rqm.user_num) {
			user_mask = BIT(rq_sta->id);
			spin_lock(&mt_lp_rq_lock);
			plat_mt_rqm.user_valid = (rq_sta->val == 0) ?
						 (plat_mt_rqm.user_valid & ~(user_mask)) :
						 (plat_mt_rqm.user_valid | user_mask);
			plat_mt_rqm.flag = MT_LP_RQ_FLAG_NEED_UPDATE;
			spin_unlock(&mt_lp_rq_lock);
		}
		break;
	default:
		break;
	}

	return MT_LP_RQ_STA_OK;
}

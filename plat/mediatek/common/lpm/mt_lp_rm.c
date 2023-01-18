/*
 * Copyright (c) 2020-2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <lpm/mt_lp_rm.h>

struct platform_mt_resource_manager {
	unsigned int count;
	struct mt_resource_manager *plat_rm;
};

static struct platform_mt_resource_manager plat_mt_rm;

int mt_lp_rm_register(struct mt_resource_manager *rm)
{
	unsigned int i;
	struct mt_resource_constraint *const *rc;

	if ((rm == NULL) || (rm->consts == NULL) ||
	    (plat_mt_rm.plat_rm != NULL)) {
		return MT_RM_STATUS_BAD;
	}

	for (i = 0U, rc = rm->consts; *rc != NULL; i++, rc++) {
		if ((*rc)->init != NULL) {
			(*rc)->init();
		}
	}

	plat_mt_rm.plat_rm = rm;
	plat_mt_rm.count = i;

	return MT_RM_STATUS_OK;
}

int mt_lp_rm_reset_constraint(unsigned int idx, unsigned int cpuid, int stateid)
{
	struct mt_resource_constraint const *rc = NULL;

	if ((plat_mt_rm.plat_rm == NULL) || (idx >= plat_mt_rm.count)) {
		return MT_RM_STATUS_BAD;
	}

	rc = plat_mt_rm.plat_rm->consts[idx];

	if ((rc == NULL) || (rc->reset == NULL)) {
		return MT_RM_STATUS_BAD;
	}

	return rc->reset(cpuid, stateid);
}

int mt_lp_rm_get_status(unsigned int type, void *priv)
{
	int res = 0;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if ((rm == NULL) || (type >= PLAT_RC_MAX)) {
		return -1;
	}

	for (con = rm->consts; *con != NULL; con++) {
		if ((*con)->get_status == NULL) {
			continue;
		}
		res = (*con)->get_status(type, priv);
		if (res == MT_RM_STATUS_STOP) {
			break;
		}
	}

	return res;
}

int mt_lp_rm_do_constraint(unsigned int constraint_id, unsigned int cpuid, int stateid)
{
	int res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint const *rc;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if ((rm == NULL) || (constraint_id >= plat_mt_rm.count)) {
		return res;
	}

	rc = rm->consts[constraint_id];
	if ((rc != NULL) && (rc->run != NULL)) {
		res = rc->run(cpuid, stateid);
	}

	return res;
}

int mt_lp_rm_find_constraint(unsigned int idx, unsigned int cpuid,
			     int stateid, void *priv)
{
	unsigned int i;
	int res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint *const *rc;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if ((rm == NULL) || (idx >= plat_mt_rm.count)) {
		return res;
	}

	/* If subsys clk/mtcmos is on, add block-resource-off flag */
	if (rm->update != NULL) {
		res = rm->update(rm->consts, plat_mt_rm.count, stateid, priv);
		if (res != 0) {
			return MT_RM_STATUS_BAD;
		}
	}

	res = MT_RM_STATUS_BAD;
	for (i = idx, rc = (rm->consts + idx); *rc != NULL; i++, rc++) {
		if (((*rc)->is_valid != NULL) &&
		    ((*rc)->is_valid(cpuid, stateid))) {
			res = i;
			break;
		}
	}

	return res;
}

int mt_lp_rm_find_and_run_constraint(unsigned int idx, unsigned int cpuid,
				     int stateid, void *priv)
{
	int res = MT_RM_STATUS_BAD;

	res = mt_lp_rm_find_constraint(idx, cpuid, stateid, priv);
	if (res != MT_RM_STATUS_BAD) {
		mt_lp_rm_do_constraint(res, cpuid, stateid);
	}

	return res;
}

int mt_lp_rm_do_update(int stateid, int type, void const *p)
{
	int res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint *const *rc;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (rm == NULL) {
		return res;
	}

	for (rc = rm->consts; *rc != NULL; rc++) {
		if ((*rc)->update != NULL) {
			res = (*rc)->update(stateid, type, p);
			if (res != MT_RM_STATUS_OK) {
				break;
			}
		}
	}

	return res;
}

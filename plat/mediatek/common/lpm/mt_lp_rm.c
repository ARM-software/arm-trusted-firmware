/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lp_rm.h>
#include <stddef.h>

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

int mt_lp_rm_reset_constraint(int idx, unsigned int cpuid, int stateid)
{
	struct mt_resource_constraint const *rc = NULL;

	if ((plat_mt_rm.plat_rm == NULL) || (idx < 0) ||
	    (idx >= plat_mt_rm.count)) {
		return MT_RM_STATUS_BAD;
	}

	rc = plat_mt_rm.plat_rm->consts[idx];

	if ((rc == NULL) || (rc->reset == NULL)) {
		return MT_RM_STATUS_BAD;
	}

	return rc->reset(cpuid, stateid);
}

int mt_lp_rm_find_and_run_constraint(int idx, unsigned int cpuid,
				     int stateid, void *priv)
{
	int i, res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint *const *rc;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if ((rm == NULL) || (idx < 0) || (idx >= plat_mt_rm.count)) {
		return res;
	}

	/* If subsys clk/mtcmos is on, add block-resource-off flag */
	if (rm->update != NULL) {
		res = rm->update(rm->consts, stateid, priv);
		if (res != 0) {
			return res;
		}
	}

	for (i = idx, rc = (rm->consts + idx); *rc != NULL; i++, rc++) {
		if (((*rc)->is_valid != NULL) &&
		    ((*rc)->is_valid(cpuid, stateid))) {
			if (((*rc)->run != NULL) &&
			    ((*rc)->run(cpuid, stateid) == 0)) {
				res = i;
				break;
			}
		}
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
